/**
 * @defgroup   BOOTLOADER findkernel
 *
 * @brief      This file finds the kernel and loads it using asm stubs.
 *
 * @author     Robbe De Greef
 * @date       2019
 */

#include <print_pm.h>
#include <types.h>

#include <fs/ext2.h>

/* Offset to the partition table */
#define PART_OFFSET 	(0x7c00 + 0x1BE) 	/* Load point of mbr and then first part offset */

/* Boot flags */
#define FLAG_ACTIVE 	(1 << 7)

/* Filesystem types */
#define FS_TYPE_EXT2 	0x01
#define FS_TYPE_FAT16 	0x02
#define FS_TYPE_FAT32 	0x03
#define FS_TYPE_EXT3 	0x04
#define FS_TYPE_EXT4 	0x05

#define KERNEL_NAME 	"kernel"

extern void jump_kernel();

/* Partiton table entry format */
#pragma pack(1)
typedef struct partition_s
{
	uint8_t 	flags;
	uint8_t 	starting_head;	/* Note that bit 6 & 7 are the upper bits of cyl */
	
	uint8_t 	start_sect;
	
	uint8_t 	starting_cyl;

	uint8_t 	system_id;
	uint8_t	 	ending_head;

	uint8_t 	ending_sect; 	/* Note that bit 6 & 7 are the upper bits of cyl */
	
	uint8_t 	ending_cyl;

	uint32_t 	part_start_lba;
	uint32_t 	total_sects;
} partition_t;


/**
 * @brief      Determines the file system.
 *
 * @param[in]  startlba     The startlba
 * @param[in]  total_sects  The total sects
 * @param[in]  disk         The disk
 * @param[in]  blocksize    The blocksize
 *
 * @return     Filesystem type
 */
int determine_fs(unsigned int startlba, unsigned int total_sects, unsigned int disk, unsigned int blocksize)
{
	/* @todo: determine filesystem */
	return FS_TYPE_EXT2;
}


/**
 * @brief      Loads the kernel.
 *
 * @param[in]  startlba     The startlba
 * @param[in]  total_sects  The total sects
 * @param[in]  disk         The disk
 * @param[in]  blocksize    The blocksize
 *
 * @return     Success on 0
 */
int load_kernel(unsigned int startlba, unsigned int total_sects, unsigned int disk, unsigned int blocksize)
{
	switch (determine_fs(startlba, total_sects, disk, blocksize)){
		case FS_TYPE_EXT2:
			load_ext2_kernel(KERNEL_NAME, startlba, total_sects, disk, blocksize);
			return 0;

		default:
			return 1;
	}
	return 0;
}

/**
 * @brief      Entry point c function
 */
void findkernel(unsigned int disk, unsigned int blocksize)
{
	/**
	 * We have to find the kernel so first we are going to parse the parition table
	 */

	print("Finding bootable partition\n");

	print("disk: "); print_int(disk); print(" bs: "); print_int(blocksize); print("\n");

	int jump = 0;

	partition_t *part_table = (partition_t*) PART_OFFSET;
	for (size_t i = 0; i < 4; i++)
	{
		/* Now we need to find the bootable partition, there can only be one active parititon so boot from the first you find*/
		if (part_table[i].flags & FLAG_ACTIVE)
		{
			print("Bootable: "); print_int(i); print("\n");

			if (load_kernel(part_table[i].part_start_lba, part_table[i].total_sects, disk, blocksize) != 0)
			{
				print("Loading kernel failed\n");
				for (;;);
			}

			print("Successfully loaded kernel\n");
			jump = 1;
			goto go_asm;
		}

	}

go_asm:
	
	if (!jump)
	{
		print("Error, no bootable partition found\nCheck if the parititon you want to boot from is actually flagged for boot");
		for(;;);
	}

	/* Jump back to asm (we cant use a normal return statement because of this compiler) */
	jump_kernel();

	/* Will never get here */
	return;
} 