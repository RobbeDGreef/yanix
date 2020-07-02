#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <fs/pipe.h>

int chardev_close(vfs_node_t *node)
{
	/* @todo: chardev_close function */
	return 0;
}

/**
 *	If the input of read or write is equal to (void*) 1 it will use the default
 * 	pipe_read and pipe_write functions respectively.
 */
vfs_node_t *chardev_create(char *path, read_fpointer read, write_fpointer write,
                           ioctl_fpointer cmd)
{
	struct pipe *pipe = pipe_create();
	if (!pipe)
		return NULL;

	if ((unsigned long) read == 1)
		read = pipe_read;

	if ((unsigned long) write == 1)
		write = pipe_write;

	vfs_node_t *node = vfs_setupnode(vfs_get_name(path), VFS_CHARDEVICE, 0, 0,
	                                 0, pipe->circbuf->size, (offset_t) pipe, 0,
	                                 chardev_close, 0, read, write, 0, 0, 0);

	node->cmd = cmd;

	vfs_link_node_vfs(path, node);
	return node;
}