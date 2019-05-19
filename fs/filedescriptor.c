#include <mm/heap.h>
#include <lib/string/string.h>
#include <drivers/vfs/vfs_node.h>
#include <fs/filedescriptor.h>
#include <stdint.h>

/*
 * this file will house the filedescriptor code 
 * the filedescriptor list will be an array for easy access and simplicity 
 * since the fd struct is not large this won't be a problem
 * this does mean that at any given time only a specific amount of files
 * can be open
*/

file_descriptor_entry_t *fd_array;

/**
 * @brief      initializes the filedescriptor table
 *
 * @return     returns 0 at success and -1 at memory allocation failture
 */
int init_filedescriptors()
{
	fd_array = (file_descriptor_entry_t*) kmalloc(sizeof(file_descriptor_entry_t)*MAX_FILEDESCRIPTORS);
	if (fd_array == 0) {
		return -1;
	}
	memset(fd_array, 0, sizeof(file_descriptor_entry_t) * MAX_FILEDESCRIPTORS);
	return 0;
}

/**
 * @brief      register a filedescriptor
 *
 * @param      inode  The inode to register
 * @param[in]  mode   The mode to register the fd in
 *
 * @return     returns the filedescriptor number at success and -1 when no fd was free
 */
int register_filedescriptor(vfs_node_t *node, int mode)
{
	for (size_t i = 0; i < MAX_FILEDESCRIPTORS; i++) {
		if (fd_array[i].node == 0){
			fd_array[i].node = node;
			fd_array[i].mode  = mode;
			return i;
		}
	}
	return -1;
}

/**
 * @brief      Closes a filedescriptor.
 *
 * @param[in]  fd    The file descriptor to close
 *
 * @return     0 when successfull, -1 when the fd was out of range
 */
int close_filedescriptor(int fd)
{
	if (fd >= 0 && fd <= MAX_FILEDESCRIPTORS) {
		memset(&fd_array[fd], 0, sizeof(file_descriptor_entry_t));
		return 0;
	} 
	return -1;
}

/**
 * @brief      Gets the filedescriptor inode.
 *
 * @param[in]  fd    file descriptor to find inode for
 *
 * @return     The filedescriptor inode or -1 when the filedesciptor cannot be found or is closed
 */
vfs_node_t *get_filedescriptor_node(int fd)
{
	if (fd >= 0 && fd <= MAX_FILEDESCRIPTORS) {
		if (fd_array[fd].node != 0){
			return fd_array[fd].node;
		}
	} 
	return (vfs_node_t*) -1;
}