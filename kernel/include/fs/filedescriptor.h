#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include <fs/vfs_node.h>

#define E_CLOEXEC 1

#define GLOBAL_FILE_TABLE_SIZE 1024
struct file_lock
{
	vfs_node_t *node; /* The vfs file node that is locked */
	pid_t       pid;  /* The process that locked the file */
};

struct file_descriptor
{
	vfs_node_t *node;
	mode_t      mode;
	int         flags;
	int         seek;
	int         lock_index;
};

int                     init_filedescriptors();
int                     close_filedescriptor(int fd);
struct file_descriptor *get_filedescriptor(int fd);
struct file_descriptor *get_filedescriptor_from_node(vfs_node_t *node);
vfs_node_t *            get_filedescriptor_node(int fd);
int                     register_filedescriptor(vfs_node_t *node, int mode);
void                    check_filedescriptors();
int                     setflags_filedescriptor(int fd, int flags);
int                     dup_filedescriptor(int fd, int from);
int                     dup2_filedescriptor(int old, int new);
void                    debug_filedescriptors();

#endif