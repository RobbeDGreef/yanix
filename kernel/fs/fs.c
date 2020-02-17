#include <stdint.h>
#include <stddef.h>
#include <fs/filedescriptor.h>
#include <fs/fs.h>

#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <fs/pipe.h>
#include <yanix/tty_dev.h>
#include <kernel.h>


/*
 * This file is the combination of all the pieces of filesystem code that together make one 
 * interface
*/

// @todo       register filesystem should not register to one global var

filesystem_t *g_current_fs;

/**
 * @brief      Reads a number of dirent structures into buffer pointed to by dir
 *
 * @param[in]  fd     The file descriptor of the opened directory
 * @param      dir    The dirent buffer
 * @param[in]  count  The size of the dirent buffer
 *
 * @return     On success, the number of bytes read is returned. On failure, -errno is returned
 */
int getdents(int fd, struct dirent *dir, int count)
{
	//vfs_node_t *node = get_filedescriptor_node(fd);
	(void) (fd);
	(void) (dir);
	(void) (count);
	return 0;

}

/**
 * @brief      A tty write function
 *
 * @param      node    The node
 * @param[in]  offset  The offset
 * @param[in]  buffer  The buffer
 * @param[in]  size    The size
 *
 * @return     { description_of_the_return_value }
 */
static ssize_t tty_stdoutwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);

	return tty_write(tty_get_device(get_current_task()->tty), buffer, size, -1, -1);
}

static ssize_t tty_stderrwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);
	tty_set_color(TTY_RED);
	int ret = tty_write(tty_get_device(get_current_task()->tty), buffer, size, -1, -1);
	tty_set_color(TTY_WHITE);
	return ret;
}

int init_char_specials()
{
	/* Create /dev/stdin */
	mkfifo("/dev/stdin");

	/* @todo: Should actually be a pipe that notifies the tty systems, because we need to be able to read from stdout too */
	vfs_node_t *stdout = vfs_setupnode("stdout", VFS_CHARDEVICE, 0, 0, 0, 0, 0, 0, 0, 0, 0, tty_stdoutwrite, 0, 0, 0);
	vfs_link_node_vfs("/dev/stdout", stdout);

	vfs_node_t *stderr = vfs_setupnode("stderr", VFS_CHARDEVICE, 0, 0, 0, 0, 0, 0, 0, 0, 0, tty_stderrwrite, 0, 0, 0);
	vfs_link_node_vfs("/dev/stderr", stderr);

	vfs_open_fd("/dev/stdin", 0, 0);
	vfs_open_fd("/dev/stdout", 0, 0);
	vfs_open_fd("/dev/stderr", 0, 0);

	return 0;
}

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
void register_filesystem(char *name, int type, fs_read_file_fpointer readfile,
						 fs_write_file_fpointer writefile, fs_open_dir_fpointer opendir, fs_read_dir_fpointer readdir, fs_make_node makenode)
{
	g_current_fs->name = name;
	g_current_fs->type = type;

	g_current_fs->file_read   = readfile;
	g_current_fs->file_write  = writefile;
	g_current_fs->dir_open    = opendir;
	g_current_fs->dir_read    = readdir;
	g_current_fs->fs_makenode = makenode;
}

