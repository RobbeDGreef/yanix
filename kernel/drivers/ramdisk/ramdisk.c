#include <drivers/disk.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <sys/types.h>

typedef struct ramdisk_info_s
{
	unsigned long location;
	unsigned long size;
} ramdisk_info_t;

/**
 * @brief      Copy the content of the ramdisk into the given buffer
 *
 * @param[in]  offset       The offset relative to ramdisk location
 * @param[in]  destination  The memory location of the buffer
 * @param[in]  size         The amount of bytes to copy
 * @param      disk_info    The disk information
 *
 * @return     Pointer to buffer location
 */
ssize_t ramdisk_read(unsigned long offset, void *destination, size_t size,
                     disk_t *disk_info)
{
	ramdisk_info_t *rd_info = (ramdisk_info_t *) disk_info->drive_info;
	return (ssize_t) memcpy((void *) destination,
	                        (void *) (rd_info->location + offset), size);
}

/**
 * @brief      Will write contents of given buffer to a given offset relative to
 *             the ramdisk location
 *
 * @param[in]  offset     The offset relative to ramdisk location to write to
 * @param[in]  buffer     The memory to copy into the ramdisk
 * @param[in]  size       The amount of bytes to copy
 * @param      disk_info  The disk information
 *
 * @return     Pointer to Buffer location
 */
ssize_t ramdisk_write(unsigned long offset, const void *buffer, size_t size,
                      disk_t *disk_info)
{
	ramdisk_info_t *rd_info = (ramdisk_info_t *) disk_info->drive_info;
	return (ssize_t) memcpy((uint32_t *) (rd_info->location + offset),
	                        (uint32_t *) buffer, size);
}

/**
 * @brief      Initializes the ramdisk
 *
 * @param[in]  location  The location of the ramdisk
 * @param[in]  size      The size of the ramdisk
 */
void init_ramdisk(offset_t location, size_t size)
{
	/* Create the ramdisk information structure */
	ramdisk_info_t *rd_info = kmalloc(sizeof(ramdisk_info_t));
	rd_info->location       = location;
	rd_info->size           = size;

	disk_t *disk_info     = kmalloc(sizeof(disk_t));
	disk_info->type       = DISK_TYPE_RAMDISK;
	disk_info->name       = "Initrd";
	disk_info->drive_info = (void *) rd_info;
	disk_info->read       = &ramdisk_read;
	disk_info->write      = &ramdisk_write;
	disk_info->next       = 0;
	disk_info->block_size = 0; /* Not necesairy */

	/* Add the ramdisk disk into the drive list */
	add_disk(disk_info);
}