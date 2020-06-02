#include <errno.h>
#include <fs/filedescriptor.h>
#include <kernel.h>
#include <yanix/ds/fd_vector.h>

struct file_lock *global_file_table;
int               lock_file(vfs_node_t *node, pid_t pid)
{
	for (int i = 0; i < GLOBAL_FILE_TABLE_SIZE; i++)
	{
		if (!global_file_table[i].node)
		{
			global_file_table[i].node = node;
			global_file_table[i].pid  = pid;
			return i;
		}
	}
	errno = ENFILE;
	return -1;
}

int find_locked_file(vfs_node_t *node)
{
	for (int i = 0; i < GLOBAL_FILE_TABLE_SIZE; i++)
	{
		if (global_file_table[i].node == node)
			return i;
	}
	return -1;
}

int unlock_file(int index)
{
	global_file_table[index].node = 0;
	return 0;
}

int register_filedescriptor(vfs_node_t *node, int mode)
{
	struct file_descriptor fd_struct;
	fd_struct.node  = node;
	fd_struct.mode  = mode;
	fd_struct.seek  = 0;
	fd_struct.flags = 0;

	return vector_add(get_current_task()->fds, fd_struct);
}

struct file_descriptor *get_filedescriptor(int fd)
{
	if (get_current_task())
		return vector_get(get_current_task()->fds, fd);

	/* @XXX: I don't think this is the right errno? */
	errno = EBADFD;
	return 0;
}

struct file_descriptor *get_filedescriptor_from_node(vfs_node_t *node)
{
	if (get_current_task())
		return vector_get_node(get_current_task()->fds, node);

	errno = EBADFD;
	return 0;
}

vfs_node_t *get_filedescriptor_node(int fd)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);

	if (!fd_struct)
		return 0;

	return fd_struct->node;
}

int close_filedescriptor(int fd)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);

	/* Checking if the filedescriptor is actually open */
	if (!fd_struct)
		return 0;

	vector_clear(get_current_task()->fds, fd);
	return 0;
}

int init_filedescriptors()
{
	return 0;
}

int dup_filedescriptor(int fd, int from)
{
	struct file_descriptor *dup = get_filedescriptor(fd);
	if (!dup)
		return -1;

	if (from == -1)
		return vector_add(get_current_task()->fds, *dup);
	else
		return vector_add_from(get_current_task()->fds, *dup, from);
}

int setflags_filedescriptor(int fd, int flags)
{
	struct file_descriptor *toset = get_filedescriptor(fd);

	if (!toset)
		return -1;

	toset->flags = flags;
	return 0;
}

void check_filedescriptors()
{
	int fds = get_current_task()->fds->vector_size;
	for (int i = 0; i < fds; i++)
	{
		struct file_descriptor *fd = get_filedescriptor(i);
		if (fd)
			if (fd->flags & E_CLOEXEC)
				close_filedescriptor(i);
	}
}
