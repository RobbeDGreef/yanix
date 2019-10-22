#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#include <fs/dirent.h>
#include <drivers/vfs/vfs_node.h>
#include <sys/types.h>

#define FS_EXT2 	0
#define FS_FAT32 	1
// ...

struct filesystem_s;
typedef struct filesystem_s filesystem_t;
struct vfs_node_s;
typedef struct vfs_node_s vfs_node_t;


typedef ssize_t (*fs_read_fpointer)  (offset_t offset, void *buf, size_t count);
typedef ssize_t (*fs_write_fpointer) (offset_t offset, const void *buf, size_t count);

typedef ssize_t			(*fs_read_file_fpointer)  (ino_t inode, void *buf, size_t count, filesystem_t *fs_info);
typedef ssize_t 		(*fs_write_file_fpointer) (ino_t inode, const void *buf, size_t count, filesystem_t *fs_info);
typedef DIR*   			(*fs_open_dir_fpointer)   (ino_t inode, filesystem_t *fs_info);
typedef int 		    (*fs_close_dir_fpointer)  (DIR* dirp);
typedef struct dirent  *(*fs_read_dir_fpointer)   (DIR* dirp);

typedef vfs_node_t 	*(*fs_make_node) (offset_t offset, id_t id, filesystem_t *fs_info);

/**
 * @brief      A filesystem information struct
 */
struct filesystem_s {
	char 				*name;
	uint32_t 			type;					// type of filesystem (ext2, fat12, NTFS, ...)
	unsigned int 		block_size;
	offset_t 			*superblock;
	offset_t 			*blockgroup_list; 		// linked list
	offset_t 			start;					// depending on implementation this could be an inode, a directory ...
	fs_read_fpointer	read;
	fs_write_fpointer	write;
	fs_read_file_fpointer 	file_read;
	fs_write_file_fpointer 	file_write;
	fs_open_dir_fpointer 	dir_open;
	fs_close_dir_fpointer 	dir_close;
	fs_read_dir_fpointer 	dir_read;
	fs_make_node 			fs_makenode;
};

extern filesystem_t *g_current_fs;


/**
 * @brief      Registers the main filesystem for use
 *
 * @param      name       The name of the fs
 * @param[in]  type       The type of filesystem (ext2, fat12, NTFS, ...)
 * @param[in]  read       The read funtion for the fs driver
 * @param[in]  write      The write function for the fs driver
 * @param[in]  readfile   The file read function pointer
 * @param[in]  writefile  The file write function pointer
 * @param[in]  opendir    The open directory stream function pointer
 * @param[in]  readdir    The read from directory stream function pointer
 * @param[in]  makenode   The make vfs node function pointer
 */
void register_filesystem(char *name, int type, fs_read_fpointer read, fs_write_fpointer write, fs_read_file_fpointer readfile,
						 fs_write_file_fpointer writefile, fs_open_dir_fpointer opendir, fs_read_dir_fpointer readdir, fs_make_node makenode);


#endif /* drivers/fs/fs.h */