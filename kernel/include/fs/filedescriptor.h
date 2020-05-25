#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#if 0

	#include <fs/vfs_node.h>

	#define MAX_FILEDESCRIPTORS 32 /* This number should probably be higher */

typedef struct {
	vfs_node_t		*node;
	int 			mode;
	int 			seek;
} file_descriptor_entry_t;


/**
 * @brief      initializes the filedescriptor table
 *
 * @return     returns 0 at success and -1 at memory allocation failture
 */
int init_filedescriptors();

/**
 * @brief      register a filedescriptor
 *
 * @param      inode  The inode to register
 * @param[in]  mode   The mode to register the fd in
 *
 * @return     returns the filedescriptor number at success and -1 when no fd was free
 */
int register_filedescriptor(vfs_node_t *inode, int mode);

/**
 * @brief      Closes a filedescriptor.
 *
 * @param[in]  fd    The file descriptor to close
 *
 * @return     0 when successfull, -1 when the fd was out of range
 */
int close_filedescriptor(int fd);

/**
 * @brief      Gets the filedescriptor inode.
 *
 * @param[in]  fd    file descriptor to find inode for
 *
 * @return     The filedescriptor inode.
 */
vfs_node_t *get_filedescriptor_node(int fd);


/**
 * @brief      Switches the filedescriptors to tty mode
 */
void switch_filedescriptors_to_tty();

int init_tty_filedescriptors();

#endif

#include <fs/vfs_node.h>

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
	int         seek;
	int         lock_index;
};

int                     init_filedescriptors();
int                     close_filedescriptor(int fd);
struct file_descriptor *get_filedescriptor(int fd);
struct file_descriptor *get_filedescriptor_from_node(vfs_node_t *node);
vfs_node_t *            get_filedescriptor_node(int fd);
int                     register_filedescriptor(vfs_node_t *node, int mode);

#endif