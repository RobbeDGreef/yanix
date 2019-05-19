/**
 * fs/open.c
 * 
 * Author: Robbe De Greef
 * Date:   19 may 2019
 * 
 * Version 1.0
 */


#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <lib/stdio/stdio.h>
#include <lib/string/string.h>
#include <fs/dirent.h>
#include <fs/filedescriptor.h>
#include <drivers/vfs/vfs.h>
#include <drivers/vfs/vfs_node.h>

/**
 * @brief      reads from given file descriptor
 *
 * @param[in]  fd     The file descriptor
 * @param[in]  buf    The buffer to read in
 * @param[in]  count  The amount of bytes to read
 *
 * @return     amount of bytes read on success and -1 if the filedescriptor
 *             could not find the file
 */
int read(int fd, void *buf, size_t count)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == (vfs_node_t*) -1){
		return -1;
	}
	return vfs_read(node, node->offset, buf, count);
}

/**
 * @brief      writes to given file descriptor
 *
 * @param[in]  fd     The file descriptor
 * @param[in]  buf    The buffer to write from
 * @param[in]  count  The amount of bytes to write
 *
 * @return     amount of bytes writen on success and -1 if the filedescriptor could not find the file 
 */
int write(int fd, const void *buf, size_t count)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == (vfs_node_t*) -1){
		return -1;
	}
	return vfs_write(node, node->offset, buf, count);
}


/**
 * @brief      closes a filedescriptor
 *
 * @param[in]  fd    the filedescriptor
 *
 * @return     returns 0 at success and -1 when the filedescriptor could not be found
 */
int close(int fd)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == (vfs_node_t*) -1){
		return -1;
	}
	vfs_close(node);
	close_filedescriptor(fd);
	return 0;
}

/**
 * @brief      Creates a file that doesn't exist
 *
 * @param[in]  path  The path
 * @param[in]  mode  The mode
 *
 * @return     { description_of_the_return_value }
 */
int creat(const char *path, int mode)
{
	
}

/**
 * @brief      opens a filedescriptor
 *
 * @param[in]  path  The path to the file to open
 * @param[in]  mode  The mode to open the file in
 *
 * @return     the filedescriptor
 */
int open(const char *path, int flags, int mode)
{
	vfs_node_t* node = vfs_find_path(path);
	if (node == 0) {
		return 0;
	}
	if (node->open != 0){
		int ret = node->open(node, flags, mode);
		if (ret == -1){
			return -1;
		}
	}
	return register_filedescriptor(node, mode);
	
}

/**
 * @brief      opens a directory
 *
 * @param[in]  path  The path to the directory
 *
 * @return     a directory stream pointer
 */
DIR* opendir(const char *path)
{
	vfs_node_t *node = vfs_find_path(path);
	if (node == 0) {
		return 0;
	}

	return node->opendir(node);
}


/**
 * @brief      reads the next entry in a directory
 *
 * @param      dirp  The directory stream pointer
 *
 * @return     the next directory entry
 */
struct dirent *readdir(DIR *dirp)
{
	return dirp->fs_info->dir_read(dirp);
}
