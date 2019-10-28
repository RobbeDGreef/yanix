#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>


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
 *
 * @return     Pointer to buffer location
 */
ssize_t ramdisk_read(offset_t offset, void *destination, size_t size);


/**
 * @brief      Will write contents of given buffer to a given offset relative to the ramdisk location
 *
 * @param[in]  offset  The offset relative to ramdisk location to write to
 * @param[in]  buffer  The memory to copy into the ramdisk
 * @param[in]  size    The amount of bytes to copy
 *
 * @return     Pointer to Buffer location
 */
ssize_t ramdisk_write(uint32_t offset, const void *buffer, uint32_t size);