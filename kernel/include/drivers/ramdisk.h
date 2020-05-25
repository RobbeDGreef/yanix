#include <drivers/disk.h>
#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>

/**
 * @brief      Initializes the ramdisk
 *
 * @param[in]  location  The location of the ramdisk
 * @param[in]  size      The size of the ramdisk
 */
void init_ramdisk(offset_t location, size_t size);

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
                     disk_t *disk_info);

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
                      disk_t *disk_info);