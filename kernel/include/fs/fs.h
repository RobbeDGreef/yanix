#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#include <drivers/disk.h>
#include <fs/dirent.h>
#include <fs/vfs_node.h>
#include <sys/types.h>

#define FS_EXT2  0
#define FS_FAT32 1
// ...

struct DIR;
struct filesystem_s;
struct vfs_node_s;
typedef struct filesystem_s filesystem_t;
typedef struct vfs_node_s   vfs_node_t;

typedef ssize_t (*fs_read_file_fpointer)(ino_t inode, unsigned int offset,
                                         void *buf, size_t count,
                                         filesystem_t *fs_info);
typedef ssize_t (*fs_write_file_fpointer)(ino_t inode, unsigned int offset,
                                          const void *buf, size_t count,
                                          filesystem_t *fs_info);
typedef DIR *(*fs_open_dir_fpointer)(ino_t inode, filesystem_t *fs_info);
typedef int (*fs_close_dir_fpointer)(DIR *dirp);
typedef struct dirent *(*fs_read_dir_fpointer)(DIR *dirp);
typedef int (*fs_update_fpointer)(vfs_node_t *newinfo);

typedef int (*fs_creat_fpointer)(vfs_node_t *node, char *name, flags_t flags);

typedef vfs_node_t *(*fs_make_node)(offset_t offset, char *name, id_t id,
                                    filesystem_t *fs_info);

/**
 * @brief      A filesystem information struct
 */
struct filesystem_s
{
	char *       name;
	uint32_t     type; // type of filesystem (ext2, fat12, NTFS, ...)
	unsigned int block_size;
	disk_t *     disk_info;
	offset_t *   superblock;
	offset_t *   blockgroup_list; // linked list
	offset_t     start; // depending on implementation this could be an inode, a
	                    // directory ...
	offset_t               partstart;
	fs_read_file_fpointer  block_read;
	fs_write_file_fpointer block_write;
	fs_creat_fpointer      create_node;
	fs_update_fpointer     update_node;
	fs_open_dir_fpointer   dir_open;
	fs_close_dir_fpointer  dir_close;
	fs_read_dir_fpointer   dir_read;
	fs_make_node           fs_makenode;
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
void register_filesystem(char *name, int type, fs_read_file_fpointer readfile,
                         fs_write_file_fpointer writefile,
                         fs_open_dir_fpointer   opendir,
                         fs_read_dir_fpointer readdir, fs_make_node makenode,
                         fs_creat_fpointer creat, fs_update_fpointer update);

int init_char_specials();
int getdents(int fd, struct dirent *dir, int count);

ssize_t fs_read(vfs_node_t *node, int seek, void *buf, size_t amount);
ssize_t fs_write(vfs_node_t *node, int seek, const void *buf, size_t amount);
int     fs_creat(vfs_node_t *node, char *path, flags_t flags);

#endif /* drivers/fs/fs.h */