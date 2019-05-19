#include <mm/heap.h>
#include <lib/string/string.h>
#include <sys/types.h>

uint32_t g_ramdisk_location = 0;
uint32_t g_ramdisk_size 	= 0;


/**
 * @brief      Initializes the ramdisk
 *
 * @param[in]  location  The location of the ramdisk
 * @param[in]  size      The size of the ramdisk
 */
void init_ramdisk(offset_t location, size_t size)
{
	g_ramdisk_size 		= size;
	g_ramdisk_location	= location;
}


/**
 * @brief      Copy the content of the ramdisk into the given buffer
 *
 * @param[in]  offset       The offset relative to ramdisk location
 * @param[in]  destination  The memory location of the buffer
 * @param[in]  size         The amount of bytes to copy
 *
 * @return     Pointer to buffer location
 */
ssize_t ramdisk_read(offset_t offset, void *destination, size_t size) 
{
	return (ssize_t) memcpy((uint32_t*) destination, (uint32_t*) (g_ramdisk_location + offset), size);
}


/**
 * @brief      Will write contents of given buffer to a given offset relative to the ramdisk location
 *
 * @param[in]  offset  The offset relative to ramdisk location to write to
 * @param[in]  buffer  The memory to copy into the ramdisk
 * @param[in]  size    The amount of bytes to copy
 *
 * @return     Pointer to Buffer location
 */
ssize_t ramdisk_write(uint32_t offset, const void *buffer, uint32_t size)
{
	return (ssize_t) memcpy((uint32_t*) (g_ramdisk_location + offset), (uint32_t*) buffer, size);
}
