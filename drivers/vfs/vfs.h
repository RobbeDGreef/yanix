#ifndef VFS_H
#define VFS_H

#include <drivers/vfs/vfs_node.h>

// end of defining

extern vfs_node_t *vfs_root;

struct file {
	vfs_node_t 		*vfs_node;
	int 			filedescriptor;
	size_t 			length;
};

/**
 * @brief      Initialises the virtual filesystem
 */
void init_vfs();

/**
 * @brief      VFS read file function
 *
 * @param      file  The opened file struct to read from
 * @param      buf   The buffer to write to
 * @param[in]  size  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read(struct file *file, void *buf, size_t size);

/**
 * @brief      VFS write file function
 *
 * @param      file    The opened file structure
 * @param      buffer  The buffer to read from
 * @param[in]  size    The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write(struct file *file, const void *buffer, size_t size);

/**
 * @brief      Opens a vfs node
 *
 * @param[in]  filepath  The filepath
 * @param[in]  flags     The flags
 * @param[in]  mode      The mode to open the node in
 *
 * @return     file structure or failture (0)
 */
struct file *vfs_open(const char *filepath, int flags, int mode);

/**
 * @brief      Closes a vfs node
 *
 * @param      file  The file structure of the opened file 
 *
 * @return     success or failure (0/-1)
 */
int vfs_close(struct file *file);

/**
 * @brief      Reads the contents from a directory stream
 *
 * @param      dirstream  The directory stream
 *
 * @return     A dirent struct
 */
struct dirent *vfs_readdir(DIR *dirstream);

/**
 * @brief      Opens a directory stream
 *
 * @param[in]  filepath  The filepath
 *
 * @return     The directory stream or an error
 */
DIR *vfs_opendir(const char *filepath);


/**
 * @brief      Finds the inode pointing to a specific path
 *
 * @param      path  The path 
 *
 * @return     The inode to look for
 */
vfs_node_t *vfs_find_path(const char *path);

/**
 * @brief      Reads from filepointer (for syscalls)
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer
 * @param[in]  amount  The amount
 *
 * @return     amount of bytes read on success
 */
ssize_t vfs_read_raw(int fd, void *buf, size_t amount);

/**
 * @brief      Writes to filepointer (for syscalls)
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer
 * @param[in]  amount  The amount
 *
 * @return     amount of bytes written on success
 */
ssize_t vfs_write_raw(int fd, const void *buf, size_t amount);

/**
 * @brief      Open a file descriptor
 *
 * @param[in]  path   The path
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode
 *
 * @return     Filedescriptor on success
 */
int vfs_open_raw(const char* path, int flags, int mode);

/**
 * @brief      Closes a filedescriptor
 *
 * @param[in]  fd    The filedescriptor
 *
 * @return     success
 */
int vfs_close_raw(int fd);

#endif
