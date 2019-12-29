#include <drivers/disk.h>
#include <mm/heap.h>
#include <libk/string.h>
#include <sys/types.h>

disk_t *disk_list = 0;

/**
 * @brief      Adds a disk to the global disk list.
 *
 * @param      disk  The disk
 */
void add_disk(disk_t *disk)
{
	if (disk_list == 0) 
	{
		disk_list = disk;
		return;
	}

	/* Cylcle through disks */
	disk_t *tmp = disk_list;
	while (tmp->next != 0)
	{
		tmp = tmp->next;
	}

	tmp->next = disk;
}

#include <debug.h>

/**
 * @brief      Read from a disk
 *
 * @note       The reason we do this check is because we need to abstract away
 *             the block addressing interface and make all the reads/writes
 *             absolute instead of LBA based.
 *
 * @param[in]  offset  The offset
 * @param      buf     The buffer
 * @param[in]  size    The size
 * @param      disk    The disk
 */
ssize_t disk_read(unsigned long offset, void *buffer, size_t size, disk_t *disk)
{
	void *buf = buffer;
	if (disk->block_size == 0)
	{
		/* This disk is already absolute */
		return disk->read(offset, buf, size, disk);
	}


	/* Calculate the correct values */
	unsigned int count  = size / disk->block_size;
	unsigned int c_rest = size % disk->block_size;

	/* LBA addresses start at 1 */
	unsigned int lba    = (offset / disk->block_size);
	unsigned int l_rest = offset % disk->block_size;
	unsigned int l_size = l_rest;

	unsigned int ret = 0;

	if (count == 0 && l_rest != 0)
	{
		l_size = c_rest;
		count = 1; /* to make sure when it is reduced by one it doesn't wrap around */
		c_rest = 0;
	}
	else if (l_rest != 0)
	{
		l_size = disk->block_size - l_rest;
		c_rest -= l_rest;
	}


	if (l_rest)
	{
		/* Create a buffer */
		void *tmp = kmalloc(disk->block_size);


		/* Read the data into the temporary buffer and set lba en count */
		disk->read(lba, tmp, 1, disk);
		lba++;
		count--;

		/* Now copy the wanted data into the real buffer */
		memcpy(buf, tmp + l_rest, l_size);

		/* Increase location of buffer */
		buf += l_size;

		/* Cleanup */
		kfree(tmp);

		/* Count read bytes */
		ret += l_size;

	}

	/* Read the main data */
	if (count)
	{
		ret += disk->read(lba, buf, count, disk);
	}


	if (c_rest)
	{
		/* Create a buffer */
		void *tmp = kmalloc(disk->block_size);

		/* Read the data into the temporary buffer */
		disk->read(lba + count, tmp, 1, disk);

		/* Now copy the wanted data into the real buffer */
		memcpy(buf + (count * disk->block_size), tmp, c_rest);
		
		/* Cleanup */
		kfree(tmp);

		/* Count read bytes */
		ret += c_rest;
	}

	return ret;
}

#include <kernel.h>

ssize_t disk_write(unsigned long offset, const void *buf, size_t size, disk_t *disk)
{
	/* @todo: Create disk write function */

	if (disk->block_size == 0)
	{
		return disk->write(offset, buf, size, disk);
	}
	else
	{
		printk("Not implemented yet (file: disk.c ~line 120) \n");
	}
	return -1;
}
