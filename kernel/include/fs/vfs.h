#ifndef VFS_H
#define VFS_H

#include <fs/vfs_node.h>

// end of defining

extern vfs_node_t *vfs_root;

struct file {
	vfs_node_t 		*vfs_node;
	int 			filedescriptor;
	size_t 			filesize;
};

/**
 * @brief      Initialises the virtual filesystem
 */
int init_vfs();


/**
 * @brief      Reads from filedescriptor
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer to write to
 * @param[in]  amount  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read_fd(int fd, void *buf, size_t amount);

/**
 * @brief      Reads from a file structure
 *
 * @param      file    The opened file struct to read from
 * @param      buf     The buffer to write to
 * @param[in]  amount  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read(struct file *file, void *buf, size_t amount);

/**
 * @brief      Writes to filedescriptor
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer to read from
 * @param[in]  amount  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write_fd(int fd, const void *buf, size_t amount);

/**
 * @brief      Writes to a file structure
 *
 * @param      file    The opened file struct to write to
 * @param      buf     The buffer to read from
 * @param[in]  amount  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write(struct file *file, const void *buf, size_t amount);


/**
 * @brief      Open a file descriptor
 *
 * @param[in]  path   The path
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode
 *
 * @return     Filedescriptor on success
 */
int vfs_open_fd(const char* path, int flags, int mode);

/**
 * @brief      Opens a vfs node
 *
 * @param[in]  path   The filepath
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode to open the node in
 *
 * @return     file structure or failture (0)
 */
struct file *vfs_open(const char *path, int flags, int mode);

/**
 * @brief      Creates file and opens it
 *
 * @param[in]  path  The path
 * @param[in]  mode  The mode
 *
 * @return     file descriptor
 */
int vfs_creat(const char *path, int mode);

/**
 * @brief      Closes a vfs node
 *
 * @param      file  The file structure of the opened file
 *
 * @return     success or failure (0/-1)
 */
int vfs_close(struct file *file);

/**
 * @brief      Closes a filedescriptor
 *
 * @param[in]  fd    The filedescriptor
 *
 * @return     success
 */
int vfs_close_fd(int fd);

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

vfs_node_t *vfs_setupnode(char *name, uint8_t type, uint16_t permissions, uid_t uid, gid_t gid, size_t size, offset_t offset,
						  open_fpointer open, close_fpointer close, creat_fpointer creat, read_fpointer read, write_fpointer write,
						  open_dir_fpointer opendir, read_dir_fpointer readdir, filesystem_t *fs_info);

/**
 * @brief      Check wheter the VFS is initialsed
 *
 * @return     True if vfs initialised
 */
int vfs_check_if_initialised();

#endif
