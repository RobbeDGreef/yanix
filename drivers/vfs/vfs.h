#ifndef VFS_H
#define VFS_H

#include <drivers/vfs/vfs_node.h>

// end of defining

extern vfs_node_t *vfs_root;


/**
 * @brief      Initialises the virtual filesystem
 */
void init_vfs();

/**
 * @brief      VFS read file function
 *
 * @param      node    The vfs node
 * @param[in]  offset  The offset (or inode) of the file
 * @param      buffer  The buffer to write to
 * @param[in]  size    The amount of bytes to read
 *
 * @return     amount of bytes read
 */
int vfs_read(vfs_node_t *node, uint32_t offset, uint8_t *buffer, size_t size);

/**
 * @brief      VFS write file function
 *
 * @param      node    The vfs node
 * @param[in]  offset  The offset (or inode) of the file
 * @param      buffer  The buffer to read from
 * @param[in]  size    The amount of bytes to write
 *
 * @return     amount of bytes written
 */
int vfs_write(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size);

/**
 * @brief      Opens a vfs node
 *
 * @param      node   The node to open
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode to open the node in
 *
 * @return     success or failure (0/-1)
 */
int vfs_open(vfs_node_t *node, int flags, int mode);

/**
 * @brief      Closes a vfs node
 *
 * @param      node  The node to close
 *
 * @return     success or failure (0/-1)
 */
int vfs_close(vfs_node_t *node);

/**
 * @brief      Reads the contents from a directory stream
 *
 * @param      node       The node pointing to the dir
 * @param      dirstream  The directory stream
 *
 * @return     A dirent struct
 */
struct dirent *vfs_readdir(vfs_node_t *node, DIR *dirstream);


/**
 * @brief      Finds the inode pointing to a specific path
 *
 * @param      path  The path 
 *
 * @return     The inode to look for
 */
vfs_node_t *vfs_find_path(const char *path);



#endif
