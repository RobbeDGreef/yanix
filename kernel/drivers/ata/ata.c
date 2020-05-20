/**
 * @defgroup   DRIVERS ata
 *
 * @brief      This file implements the ata driver.
 *
 * @author     Robbe De Greef
 * @date       2019
 */

#include <drivers/pci.h>
#include <drivers/disk.h>
#include <kernel.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <cpu/io.h>
#include <yanix/kfunctions.h>

#include <debug.h>

#include <drivers/ata.h>

/* ATA registers */
#define ATA_REG_DATA                0x00
#define ATA_REG_ERROR               0x01
#define ATA_REG_FEATURES            0x01
#define ATA_REG_SECCOUNT0           0x02
#define ATA_REG_LBA0                0x03
#define ATA_REG_LBA1                0x04
#define ATA_REG_LBA2                0x05
#define ATA_REG_HDDEVSEL            0x06
#define ATA_REG_CMD                 0x07
#define ATA_REG_STATUS              0x07
#define ATA_REG_SECCOUNT1           0x08
#define ATA_REG_LBA3                0x09
#define ATA_REG_LBA4                0x0A
#define ATA_REG_LBA5                0x0B
#define ATA_REG_CONTROL             0x0C
#define ATA_REG_ALTSTATUS           0x0C
#define ATA_REG_DEVADDRESS 			0x0D

#define ATA_ERROR_BBK               0x080   /* Bad block */
#define ATA_ERROR_UNC               0x040   /* Uncorrectable data */
#define ATA_ERROR_MC                0x020   /* Media changed */
#define ATA_ERROR_IDNF              0x010   /* ID mark not found */
#define ATA_ERROR_MCR               0x008   /* Media change request */
#define ATA_ERROR_ABRT              0x004   /* Command aborted */
#define ATA_ERROR_TK0NF             0x002   /* Track 0 not found */
#define ATA_ERROR_AMNF 				0x001  	/* No address mark */

/* ATA commands */
#define ATA_CMD_RESET               0x04
#define ATA_CMD_READ_SECTORS        0x20
#define ATA_CMD_READ_SECTORS_EXT    0x24
#define ATA_CMD_WRITE_SECTORS       0x30
#define ATA_CMD_WRITE_SECTORS_EXT   0x34
#define ATA_CMD_READ_DMA            0xC8
#define ATA_CMD_READ_DMA_EXT        0x25
#define ATA_CMD_WRITE_DMA           0xCA
#define ATA_CMD_WRITE_DMA_EXT       0x35
#define ATA_CMD_CACHE_FLUSH         0xE7
#define ATA_CMD_CACHE_FLUSH_EXT     0xEA
#define ATA_CMD_PACKET              0xA0
#define ATA_CMD_IDENTIFY_PACKET     0xA1
#define ATA_CMD_IDENTIFY 			0xEC

#define ATA_STATUS_BSY              0x80    /* Busy */
#define ATA_STATUS_DRDY             0x40    /* Drive ready */
#define ATA_STATUS_DF               0x20    /* Drive write fault */
#define ATA_STATUS_DSC              0x10    /* Drive seek complete */
#define ATA_STATUS_DRQ              0x08    /* Data request ready */
#define ATA_STATUS_CORR             0x04    /* Corrected data */
#define ATA_STATUS_IDX              0x02    /* Index */
#define ATA_STATUS_ERR 				0x01 	/* Error */

#define ATA_MODE_CHS                0x10
#define ATA_MODE_LBA28              0x20
#define ATA_MODE_LBA48 				0x40

#define ATA_PIO_PORT_P_BASE			0x1F0
#define ATA_PIO_PORT_P_CTRL			0x3F6
#define ATA_PIO_PORT_S_BASE 		0x170
#define ATA_PIO_PORT_S_CTRL			0x376

#define ATA_CAPABILITIES_LBA 		0x200

#define ATA_PRIMARY      			0x00
#define ATA_SECONDARY    			0x01

#define ATA_IDENT_DEVICETYPE        0
#define ATA_IDENT_CYLINDERS         2
#define ATA_IDENT_HEADS             6
#define ATA_IDENT_SECTORS           12
#define ATA_IDENT_SERIAL            20
#define ATA_IDENT_MODEL             54
#define ATA_IDENT_CAPABILITIES      98
#define ATA_IDENT_FIELDVALID        106
#define ATA_IDENT_MAX_LBA           120
#define ATA_IDENT_COMMANDSETS       164
#define ATA_IDENT_MAX_LBA_EXT 		200

#define ATA_TYPE_PATA 				0x0000
#define ATA_TYPE_SATA 				0xC33C
#define ATA_TYPE_PATAPI 			0xEB14
#define ATA_TYPE_SATAPI				0x9669

#define ATA_COMMAND_SET_48			(1 << 26)



int 		selected_mode 	= 0;
ata_drive_t *selected_drive = 0;
ata_drive_t ata_drives[4];


/**
 * @brief      DEBUG: ATA error code to string
 *
 * @param[in]  error  The error
 *
 * @return     String pointer
 */
char *ata_error_to_string(int error)
{
	switch (error)
	{
		case ATA_ERROR_BBK: 	return "Bad block";
		case ATA_ERROR_UNC: 	return "Uncorrectable data";
		case ATA_ERROR_MC: 		return "Media changed";
		case ATA_ERROR_IDNF:	return "ID mark not found";
		case ATA_ERROR_MCR:		return "Media change request";
		case ATA_ERROR_ABRT:	return "Command aborted";
		case ATA_ERROR_TK0NF:	return "Track 0 not found";
		case ATA_ERROR_AMNF:	return "No address mark";
		default:				return "Unknown";
	}
}


/**
 * @brief      Waits for a drive
 *
 * @param      drive  The drive
 */
static void ata_wait(unsigned long base)
{
	for (size_t i = 0; i < 4; i++)
		port_byte_in(base + ATA_REG_ALTSTATUS);
}

/**
 * @brief      Waits until the busy bit clears
 *
 * @param[in]  base  The base register
 *
 * @return     Status byte 
 */
static uint8_t ata_wait_clear_bsy(unsigned long base)
{
	uint8_t ret;
	while ((ret = port_byte_in(base + ATA_REG_STATUS)) & ATA_STATUS_BSY);
	return ret;
}

/**
 * @brief      Clears the error bit by sending 0 to error register
 *
 * @param      drive  The drive
 */
static void clear_error(ata_drive_t *drive)
{
	port_byte_out(drive->base + ATA_REG_ERROR, 0);
}

/**
 * @brief      Selects a drive
 *
 * @param      drive  The drive
 * @param[in]  mode   The mode
 */
static void ata_select_drive(ata_drive_t *drive, int mode)
{
	/* Check if the drive is already selected */
	if (drive != selected_drive && mode != selected_mode)
	{
		if (drive->capabilities & ATA_CAPABILITIES_LBA)
		{
			port_byte_out(drive->base + ATA_REG_HDDEVSEL, 0xE0 | (drive->slave << 4) | (mode & 0xF));
			//port_byte_out(drive->base + ATA_REG_HDDEVSEL, 0xE0 | (drive->slave << 4));
		}
		else
		{
			printk("CHS\n");
			port_byte_out(drive->base + ATA_REG_HDDEVSEL, 0xA0 | (drive->slave << 4) | (mode & 0xF));
		}

		for (size_t i = 0; i < 5; i++)
		{
			ata_wait(drive->base);
		}

		selected_drive = drive;
		selected_mode = mode;
	}
}

/**
 * @brief      Software reset
 *
 * @param[in]  base  The base register
 * @param[in]  ctrl  The control register
 */
static void ata_software_reset(unsigned long base, unsigned long ctrl)
{
	/* Send reset command */
	port_byte_out(ctrl, ATA_CMD_RESET);

	/* Wait */
	ata_wait(base);

	/* Send 0 byte */
	port_byte_out(ctrl, 0);
}

/**
 * @brief      Polling until drive is ready again.
 *
 * @param      drive           The drive
 * @param[in]  advanced_check  The advanced check
 *
 * @return     error code
 */
static int ata_polling(unsigned long base, int advanced_check)
{
	/* 400 ns delay */
	ata_wait(base);

	/* Block until BSY is cleared */
	int status = (int) ata_wait_clear_bsy(base);

	if (advanced_check)
	{
		if ((status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF))
		{
			/* Wait for the drive */
			ata_wait(base);

			uint8_t error = port_byte_in(base + ATA_REG_ERROR);
			
			printk(KERN_DEBUG "ATA drive error: %s\n", ata_error_to_string(error));
			return -error;
		}

		if (!(status & ATA_STATUS_DRQ))
		{
			printk(KERN_DEBUG "ATA drq not set\n");
			return -1;
		}
	}
	return 0;
}

/* inline */static void ata_send_sec_and_lba(ata_drive_t *drive, unsigned long lba, unsigned long count)
{
	#if 0
	/* @todo: head should be calculated with all of these see osdev pci ide */
	if (drive->mode == ATA_MODE_CHS)
	{
		unsigned int sect = (lba % 63) + 1;
		unsigned int cyl  = (lba + 1 - sect) / (16 * 63);
		
		port_byte_out(drive->base + ATA_REG_SECCOUNT0, count & 0xFF);
		port_byte_out(drive->base + ATA_REG_LBA0, (uint8_t) sect);
		port_byte_out(drive->base + ATA_REG_LBA1, (uint8_t) (cyl & 0xFF));
		port_byte_out(drive->base + ATA_REG_LBA2, (uint8_t) ((cyl >> 8) & 0xFF));

	}
	else
	{
		if (drive->mode == ATA_MODE_LBA48)
		{
			/* Send sector count */
			port_byte_out(drive->base + ATA_REG_SECCOUNT0, (count >> 8) & 0xFF);

			printk(KERN_WARNING "Oh oh looks like you found a part of code that doesn't work due to a bug. (ata.c) ata_send_sec_and_lba\n");
			/* Send LBA in pieces */
			port_byte_out(drive->base + ATA_REG_LBA0, (uint8_t) ((lba >> (8 * 3)) & 0xFF));
			
			/* NOTE: these are commented out because LBA couldn't be a uint64_t because of compiler bug and all */
			//port_byte_out(drive->base + ATA_REG_LBA1, (uint8_t) ((lba >> (8 * 4)) & 0xFF));
			//port_byte_out(drive->base + ATA_REG_LBA2, (uint8_t) ((lba >> (8 * 5)) & 0xFF));
		}
		
		/* Now send the first part of the LBA and sector count */
		port_byte_out(drive->base + ATA_REG_SECCOUNT0, count & 0xFF);
		port_byte_out(drive->base + ATA_REG_LBA0, (uint8_t) (lba) & 0xFF);
		port_byte_out(drive->base + ATA_REG_LBA1, (uint8_t) ((lba >> 8)  & 0xFF));
		port_byte_out(drive->base + ATA_REG_LBA2, (uint8_t) ((lba >> 16) & 0xFF));
	}

	#endif
	if (drive->mode == ATA_MODE_LBA48)
	{
		/* Send sector count */
		printk(KERN_WARNING "Oh oh looks like you found a part of code that doesn't work due to a bug. (ata.c) ata_send_sec_and_lba\n");
		port_byte_out(drive->base + ATA_REG_SECCOUNT1, count & 0xFF00);
		/* Send LBA in pieces */
		//port_short_out(drive->base + ATA_REG_LBA0, (uint16_t) ((lba >> (8 * 3)) & 0xFF));
		port_byte_out(drive->base + ATA_REG_LBA3, (uint8_t) 0);
		port_byte_out(drive->base + ATA_REG_LBA4, (uint8_t) 0);
		port_byte_out(drive->base + ATA_REG_LBA5, (uint8_t) 0);
		/* NOTE: these are commented out because LBA couldn't be a uint64_t because of compiler bug and all */
		//port_byte_out(drive->base + ATA_REG_LBA1, (uint8_t) ((lba >> (8 * 4)) & 0xFF));
		//port_byte_out(drive->base + ATA_REG_LBA2, (uint8_t) ((lba >> (8 * 5)) & 0xFF));
	}
	/* Now send the first part of the LBA and sector count */
	port_byte_out(drive->base + ATA_REG_SECCOUNT0, count & 0xFF);
	port_byte_out(drive->base + ATA_REG_LBA0, (uint8_t) (lba)         );
	port_byte_out(drive->base + ATA_REG_LBA1, (uint8_t) ((lba >> 8)  ));
	port_byte_out(drive->base + ATA_REG_LBA2, (uint8_t) ((lba >> 16) ));

}

/**
 * @brief      Reads from pata drive (atapio)
 *
 * @param      drive   The drive
 * @param[in]  lba     The lba
 * @param[in]  count   The count
 * @param      buffer  The buffer
 *
 * @return     error code
 */
ssize_t _atapio_read(ata_drive_t *drive, unsigned long lba, uint16_t *buffer, size_t count)
{

	/*@BUG: Oke so due to a compiler bug the lba variable above can't be uint64_t (and thus the note below) */
	size_t retry_count = 0;

retry:

	if (++retry_count == 5)
	{
		errno = EIO;
		return -1;
	}

	if (drive->capabilities & ATA_CAPABILITIES_LBA)
	{
		/* This is a LBA drive, select the correct LBA mode */
		ata_select_drive(drive, drive->mode | ((lba >> 24) & 0xF));

		/* Clear error */
		clear_error(drive);
		
		/* Send sector count and lba address */
		ata_send_sec_and_lba(drive, lba, count);

		/* Wait for the drive */
		ata_wait(drive->base);

		if (drive->mode == ATA_MODE_LBA48)
		{
			/* If our drive is in LBA48 mode, read extended sectors */
			port_byte_out(drive->base + ATA_REG_CMD, ATA_CMD_READ_SECTORS_EXT);
		}
		else
		{
			/* If our drive is just regular LBA 28 mode, read sectors normally */
			port_byte_out(drive->base + ATA_REG_CMD, ATA_CMD_READ_SECTORS);
		}

		/* Wait for the drive */
		ata_wait(drive->base);

		for (size_t cnt = count; cnt != 0; cnt--)
		{
			int poll_ret = ata_polling(drive->base, 1);			

			/* If poll_ret is not zero then an error was returned */
			if (poll_ret)
			{
				if (poll_ret == -ATA_ERROR_ABRT)
				{
					/* Retry the read */
					printk(KERN_DEBUG "Ata retrying...\n");

					for (size_t j = 0; j < 5; j++)
						ata_wait(drive->base);

					goto retry;
				}
			}

			for (size_t i = 0; i < 256; i++)
			{
				uint16_t data = port_word_in(drive->base + ATA_REG_DATA);
				buffer[i] = data;
				
			}
			buffer += 256;

			ata_wait(drive->base);
			ata_polling(drive->base, 0);

			/* Send the cache flush command */
			port_byte_out(drive->base + ATA_REG_DATA, ATA_CMD_CACHE_FLUSH);
			ata_polling(drive->base, 0);
		}

		if (ata_polling(drive->base, 0))
		{
			errno = EINVAL;
			return -1;
		}

	}
	else
	{
		/* This is a CHS drive */
		/* @todo: Implement CHS drive */
		printk(KERN_WARNING "atapio chs mode is not supported yet.\n");
	}
	//printk("END Bsy bit is %i\n", port_byte_in(drive->base+ATA_REG_STATUS) & ATA_STATUS_BSY);
	//printk("BSY BIT: %i\n", port_byte_in(0x1f0+7) & 0x80);

	return 0;
}

ssize_t _atapio_write(ata_drive_t *drive, unsigned long lba, const void *buffer, size_t count)
{
	/* Select drive */
	ata_select_drive(drive, drive->mode);

	/* Clear error port */
	clear_error(drive);

	/* Send sector count and LBA offset */
	ata_send_sec_and_lba(drive, lba, count);

	/* Wait for drive */
	ata_wait(drive->base);

	/* Send write sector command */
	if (drive->mode == ATA_MODE_LBA48)
	{
		/* If our drive is in LBA48 mode, read extended sectors */
		port_byte_out(drive->base + ATA_REG_CMD, ATA_CMD_WRITE_SECTORS_EXT);
	}
	else
	{
		/* If our drive is just regular LBA 28 mode, read sectors normally */
		port_byte_out(drive->base + ATA_REG_CMD, ATA_CMD_WRITE_SECTORS);
	}

	/* Set buffer to uint16_t */
	uint16_t *buf = (uint16_t *) buffer;

	/* For each sector (sectorcount) */
	for (size_t seccnt = 0; seccnt < count; seccnt++)
	{
		/* Check if DRQ is set*/
		if (ata_polling(drive->base, 1))
		{
			errno = EINVAL;
			return -1;
		}

		for (size_t i = 0; i < 256; i++)
		{
			port_word_out(drive->base + ATA_REG_DATA, buf[i]);
		}

		buf = (uint16_t*) (((unsigned int) buf) + 512);
	
	}
	
	/* Wait until the drive is ready again */
	ata_polling(drive->base, 0);

	/* Send the cache flush command */
	port_byte_out(drive->base + ATA_REG_DATA, ATA_CMD_CACHE_FLUSH);
	ata_polling(drive->base, 0);
	
	return 0;

}

ssize_t atapio_read(unsigned long offset, void *buffer, size_t count, disk_t *disk_info)
{
	int ret = _atapio_read((ata_drive_t*) disk_info->drive_info, offset, buffer, count);
	if (ret == 0)
		return count;
	else
		return -1;
}

ssize_t atapio_write(unsigned long offset, const void *buffer, size_t count, disk_t *disk_info)
{
	int ret = _atapio_write((ata_drive_t*) disk_info->drive_info, offset, buffer, count);
	
	if (ret == 0)
		return count;
	else
		return -1;
}


#include <debug.h>
#include <libk/string.h>
#include <mm/heap.h>

/**
 * @brief      Initializes the ata.
 *
 * @param      pci_dev  The pci dev
 *
 * @return     error code
 */
int init_ata(pci_device_t *pci_dev)
{
	pci_common_header_t *pci_hdr = (pci_common_header_t*) pci_dev->header;

	/* Loops means primary and secondary channels */
	for (size_t channel = 0; channel < 2; channel++)
	{
		/* First find the base and ctrl ports */
		unsigned long base = pci_hdr->BAR[channel];
		unsigned long ctrl = pci_hdr->BAR[channel+1];
		
		/* If the channels were not specifically specified set them here to default */
		if (base == 0 || base == 1)
		{
			base = channel ? ATA_PIO_PORT_S_BASE : ATA_PIO_PORT_P_BASE;
		}

		if (ctrl == 0 || ctrl == 1)
		{
			ctrl = channel ? ATA_PIO_PORT_S_CTRL : ATA_PIO_PORT_P_CTRL;
		}

		/* Turn off IRQ's */
		port_byte_out(ctrl + ATA_REG_CONTROL, 2);

		/* Loop means master and slave */
		for (size_t ms = 0; ms < 2; ms++)
		{
			/* Wait until master drive is ready */	
			ata_software_reset(base, ctrl);

			/* Select drive */
			port_byte_out(base + ATA_REG_HDDEVSEL, 0xA0 | ms << 4);
			
			/* Wait for drive again */
			ata_wait(base);

			/* Get the signature bytes */
			uint8_t type_low = port_byte_in(base + ATA_REG_LBA1);
			uint8_t type_hi  = port_byte_in(base + ATA_REG_LBA2);

			/* Combine them for easier reading */
			uint16_t type = (type_hi << 8) | type_low;

			if (type != ATA_TYPE_PATA)
			{
				printk("ATA device of type %x is not supported\n", type);
				/* Not supported so we're not gonna put it in the drive list */
				break;
			}

			/* Now run the identify command */

			/* Create a buffer */
			char *ident_buffer = kmalloc(512);
			memset(ident_buffer, 0, 512);

			/**
			 * @NOTE: if we ever support anything else then PATA keep in mind that this ata cmd can be different
			 * 	for example PATAPI and SATAPI use ATA_CMD_INDENTIFY_EXT 
			 */  
			/* Send identify command */
			port_byte_out(base + ATA_REG_CMD, ATA_CMD_IDENTIFY);

			/* Check whether the drive exits */
			if (port_byte_in(base + ATA_REG_STATUS) == 0)
			{
				/* Cleanup */
				kfree(ident_buffer);
				break;
			}

			/* Wait for the disk */
			ata_polling(base, 1);

			/* Copy the data */
			for (size_t i = 0; i < 512; i += 2)
			{
				uint16_t data = port_word_in(base + ATA_REG_DATA);
				ident_buffer[i+0] = data & 0xFF;
				ident_buffer[i+1] = (data >> 8) & 0xFF;
			}

			/* At this point we can create a drive structure because we are sure it exists */
			ata_drive_t *ata_info = kmalloc(sizeof(ata_drive_t));
			ata_info->base  = base;
			ata_info->ctrl  = ctrl;
			ata_info->type  = type;
			ata_info->id    = (channel * 2 + ms); 
			ata_info->slave = ms;

			ata_info->signature    = *(uint16_t*) (ident_buffer + ATA_IDENT_DEVICETYPE);
			ata_info->capabilities = *(uint16_t*) (ident_buffer + ATA_IDENT_CAPABILITIES);
			ata_info->command_set  = *(uint32_t*) (ident_buffer + ATA_IDENT_COMMANDSETS);

			printk("ata info loc: %x\n", ata_info);

			if (ata_info->command_set & ATA_COMMAND_SET_48)
			{
				ata_info->mode = ATA_MODE_LBA48;
				/* @todo: Max lba needs to be calculated here */
			} 
			else
			{
				ata_info->mode = ATA_MODE_LBA28;
				ata_info->max_lba = *(uint32_t*)(ident_buffer + ATA_IDENT_MAX_LBA);
			}

			ata_info->mode = ATA_MODE_LBA28;

			/* Now set model name and it's stored in a very weird way */
			for (size_t i = 0; i < 40; i += 2)
			{
				ata_info->model[i]     = ident_buffer[ATA_IDENT_MODEL + i + 1];
				ata_info->model[i + 1] = ident_buffer[ATA_IDENT_MODEL + i];
			}

			/* Cleaup */
			kfree(ident_buffer);

			/* Now finally create the disk structure and add it in */
			disk_t *ata_disk = kmalloc(sizeof(disk_t));

			ata_disk->type = DISK_TYPE_HARDDISK;
			ata_disk->name = ata_info->model;
			ata_disk->drive_info = (void*) ata_info;
			ata_disk->block_size = 512;
			ata_disk->read  = &atapio_read;
			ata_disk->write = &atapio_write;
			ata_disk->next  = 0;

			add_disk(ata_disk);

			printk("ATA %i added\n", ata_info->id);
		}

	}
	return 0;
}
