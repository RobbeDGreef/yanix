#include <mm/heap.h>
#include <libk/string.h>

#include <fs/vfs_node.h>
#include <fs/vfs.h>
#include <fs/filedescriptor.h>
#include <stdint.h>
#include <errno.h>
#include <yanix/tty_dev.h>
#include <proc/tasking.h>

/*
 * this file will house the filedescriptor code 
 * the filedescriptor list will be an array for easy access and simplicity 
 * since the fd struct is not large this won't be a problem
 * this does mean that at any given time only a specific amount of files
 * can be open
*/

extern task_t *g_runningtask;
file_descriptor_entry_t *fd_array;

/* Function from stdin.c */
ssize_t tty_stdinwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size);
ssize_t tty_stdinread(vfs_node_t *node, uint32_t offset, void *buffer, size_t size);

/**
 * @brief      A tty write function
 *
 * @param      node    The node
 * @param[in]  offset  The offset
 * @param[in]  buffer  The buffer
 * @param[in]  size    The size
 *
 * @return     { description_of_the_return_value }
 */
static ssize_t tty_stdoutwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);

	return tty_write(tty_get_device(g_runningtask->tty), buffer, size, -1, -1);
}

static ssize_t tty_stderrwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);
	tty_set_color(TTY_RED);
	int ret = tty_write(tty_get_device(g_runningtask->tty), buffer, size, -1, -1);
	tty_set_color(TTY_WHITE);
	return ret;
}

/**
 * @brief      Initializes the io file descriptor
 */
void init_tty_filedescriptors()
{
	// @todo: create a system to read from the inputted data
	vfs_node_t *stdin = vfs_setupnode("stdin", VFS_CHARDEVICE, 0, 0, 0, 0, 0, 0, 0, 0, &tty_stdinread, &tty_stdinwrite, 0, 0, 0);
	register_filedescriptor(stdin, 0);
	vfs_node_t *stdout = vfs_setupnode("stdout", VFS_CHARDEVICE, 0, 0, 0, 0, 0, 0, 0, 0, 0, &tty_stdoutwrite, 0, 0, 0);
	register_filedescriptor(stdout, 0);
	vfs_node_t *stderr = vfs_setupnode("stderr", VFS_CHARDEVICE, 0, 0, 0, 0, 0, 0, 0, 0, 0, &tty_stderrwrite, 0, 0, 0);
	register_filedescriptor(stderr, 0);
}

/**
 * @brief      Switches the filedescriptors to tty mode
 */
void switch_filedescriptors_to_tty()
{
	get_filedescriptor_node(0)->read  = 0;
	get_filedescriptor_node(1)->write = &tty_stdoutwrite;
	get_filedescriptor_node(2)->write = &tty_stderrwrite;
}

/* @TODO: a per task filedescriptor list should be created */

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
			fd_array[i].mode = mode;
			return i;
		}
	}
	// too many open files so EMFILE error
	errno = EMFILE;
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
	/**
	 * We start from 3 because the first 3 are IO registers and yes they
	 * should be defined per task i know 
	 */
	if (fd >= 3 && fd <= MAX_FILEDESCRIPTORS) {
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
	if (fd_array != 0 && fd >= 0 && fd <= MAX_FILEDESCRIPTORS) {
		if (fd_array[fd].node != 0){
			return fd_array[fd].node;
		}
	} 
	errno = ENXIO;
	return 0;
}