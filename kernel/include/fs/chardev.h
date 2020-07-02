#ifndef FS_CHARDEV_H
#define FS_CHARDEV_H

#include <fs/vfs_node.h>

/**
 *	If the input of read or write is equal to 1 it will use the default
 * 	pipe_read and pipe_write functions respectively.
 */
vfs_node_t *chardev_create(char *path, read_fpointer read, write_fpointer write,
                           ioctl_fpointer cmd);

#endif /* FS_CHARDEV_H */