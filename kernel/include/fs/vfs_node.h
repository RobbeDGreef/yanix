#ifndef VFS_NODE_H
#define VFS_NODE_H

#include <fs/dirent.h>
#include <fs/fs.h>
#include <sys/types.h>

struct DIR_s;
typedef struct DIR_s DIR;

struct filesystem_s;
typedef struct filesystem_s filesystem_t;

#include <stddef.h>
#include <stdint.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE        0x05
#define VFS_SYMLINK     0x06

#define VFS_MOUNTPOINT 0x08
#define VFS_SOCKET     0x09

#define VFS_READ  0
#define VFS_WRITE 1

struct dirent;
typedef struct vfs_node_s vfs_node_t;

typedef ssize_t (*read_fpointer)(vfs_node_t *, unsigned int offset,
                                 void *buffer, size_t size);
typedef ssize_t (*write_fpointer)(vfs_node_t *, unsigned int offset,
                                  const void *buffer, size_t size);
typedef int (*close_fpointer)(vfs_node_t *);
typedef int (*open_fpointer)(vfs_node_t *, int already_exists, int flags,
                             int mode);
typedef int (*creat_fpointer)(vfs_node_t *, char *name, flags_t flags);

typedef struct dirent *(*read_dir_fpointer)(DIR *dirstream);
typedef DIR *(*open_dir_fpointer)(vfs_node_t *);
typedef int (*close_dir_fpointer)(DIR *dirstream);
typedef int (*ioctl_fpointer)(int request, char *args);

/**
 * @brief      A virtual filesystem node
 */
struct vfs_node_s
{
	char *   name; /* Name of the node */
	uint8_t  type; // directory (0) , normal file (1) , special file (2), ...
	uint16_t permissions; // node permissions
	uid_t    uid;         // user id
	gid_t    gid;         // group id
	id_t     id;          // node id
	size_t   filelength;  // length of file in bytes
	filesystem_t
		*    fs_info; // the information of the filesystem this inode points to
	offset_t offset;  // this points to the offset of the file (this can also be
	                  // something like an inode depending on implementation)
	                  // basically any way to refrence a file
	nlink_t nlink;    /* Number of hard links */

	/* These pointers are normally only used to override fs functions */
	open_fpointer  open;  // open file descriptor
	close_fpointer close; // close file descriptor

	read_fpointer  read;  /* read from file */
	write_fpointer write; /* write to file */
	creat_fpointer creat; /* create a file */
	ioctl_fpointer cmd; /* Send a command to the filedescriptor / char device */

	open_dir_fpointer  opendir;
	close_dir_fpointer closedir;
	read_dir_fpointer  readdir; // returns amount of files

	vfs_node_t *ptr;      // for mountpoints and symlinks
	vfs_node_t *nextnode; // linked list next node
	vfs_node_t *dirlist;  // if this is a directory this will point to a linked
	                      // list of vfs_node_t contents of this dir
	vfs_node_t *parent;   // pointer to parent
};

#endif