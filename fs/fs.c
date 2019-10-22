#include <stdint.h>
#include <stddef.h>
#include <fs/filedescriptor.h>
#include <fs/fs.h>


/*
 * This file is the combination of all the pieces of filesystem code that together make one 
 * interface
*/

// @todo       register filesystem should not register to one global var

filesystem_t *g_current_fs;


/**
 * @brief      Registers the main filesystem for use
 *
 * @param      name       The name of the fs
 * @param[in]  type       The type of filesystem (ext2, fat12, NTFS, ...)
 * @param[in]  read       The read funtion for the fs driver
 * @param[in]  write      The write function for the fs driver
 * @param[in]  readfile   The file read function pointer
 * @param[in]  writefile  The file write function pointerw
 * @param[in]  opendir    The open directory stream function pointer
 * @param[in]  readdir    The read from directory stream function pointer
 * @param[in]  makenode   The make vfs node function pointer
 */
void register_filesystem(char *name, int type, fs_read_fpointer read, fs_write_fpointer write, fs_read_file_fpointer readfile,
						 fs_write_file_fpointer writefile, fs_open_dir_fpointer opendir, fs_read_dir_fpointer readdir, fs_make_node makenode)
{
	g_current_fs->name = name;
	g_current_fs->type = type;
	g_current_fs->read = read;
	g_current_fs->write = write;
	g_current_fs->file_read   = readfile;
	g_current_fs->file_write  = writefile;
	g_current_fs->dir_open    = opendir;
	g_current_fs->dir_read    = readdir;
	g_current_fs->fs_makenode = makenode;
}

