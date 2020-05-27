#include <fs/filedescriptor.h>
#include <fs/fs.h>
#include <stddef.h>
#include <stdint.h>

#include <errno.h>
#include <fs/pipe.h>
#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <kernel.h>
#include <libk/stdio.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <yanix/tty_dev.h>

#include <drivers/serial.h>

/*
 * This file is the combination of all the pieces of filesystem code that
 * together make one interface
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
 * @return     On success, the number of bytes read is returned. On failure,
 * -errno is returned
 */
int getdents(int fd, struct dirent *dir, int count)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);

	if (!fd_struct)
		return -1;

	/* Whenever a directory is opened, the fd's seek is a pointer to the yanix
	 * DIR struct */
	DIR *dirstream = (DIR *) fd_struct->seek;

	if (!dirstream)
	{
		errno = EBADFD;
		return -1;
	}

	int            i      = 0;
	char *         buf    = (char *) dir;
	struct dirent *dirent = 0;

	while (i < count)
	{
		dirent = vfs_readdir(dirstream);

		// printk_hd(dirent, dirent->d_reclen);

		/* The directory is completely read if 0 is returned */
		if (!dirent)
			break;

		i += dirent->d_reclen;

		if (i <= count)
		{
			memcpy(buf, dirent, dirent->d_reclen);
			((struct dirent *) buf)->d_off = i;
			buf += dirent->d_reclen;
		}
		else
		{
			i -= dirent->d_reclen;
			break;
		}
	}

	return i;
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
static ssize_t tty_stdoutwrite(vfs_node_t *node, unsigned int offset,
                               const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);

	for (unsigned int i = 0; i < size; i++)
		serial_put(((char *) buffer)[i]);

	return tty_write(tty_get_device(get_current_task()->tty), buffer, size, -1,
	                 -1);
}

#include <debug.h>
static ssize_t tty_stderrwrite(vfs_node_t *node, unsigned int offset,
                               const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);
	for (unsigned int i = 0; i < size; i++)
		serial_put(((char *) buffer)[i]);

	tty_set_color(TTY_RED);

	int ret = tty_write(tty_get_device(get_current_task()->tty), buffer, size,
	                    -1, -1);
	tty_set_color(TTY_WHITE);

	return ret;
}

char *stdinbuffer;
int   index;

static ssize_t tty_stdinwrite(vfs_node_t *node, unsigned int offset,
                              const void *buffer, size_t size)
{
	/* @todo: we need to properly parse the stdin */
	if (*(char *) buffer == 0x8)
	{
		if (!pipe_remove(node, offset, 1))
			tty_stdoutwrite(0, 0, buffer, size);
		return 0;
	}

	pipe_write(node, offset, buffer, size);
	tty_stdoutwrite(0, 0, buffer, size);

	return size;
}

int init_char_specials()
{
	/* Create /dev/stdin */
	stdinbuffer = kmalloc(BUFSIZ);
	mkfifo("/dev/stdin");

	vfs_node_t *node = vfs_find_path("/dev/stdin");
	if (node)
		node->write = &tty_stdinwrite;

	/* @todo: Should actually be a pipe that notifies the tty systems, because
	 * we need to be able to read from stdout too */
	vfs_node_t *stdout = vfs_setupnode("stdout", VFS_CHARDEVICE, 0, 0, 0, 0, 0,
	                                   0, 0, 0, 0, tty_stdoutwrite, 0, 0, 0);
	vfs_link_node_vfs("/dev/stdout", stdout);

	vfs_node_t *stderr = vfs_setupnode("stderr", VFS_CHARDEVICE, 0, 0, 0, 0, 0,
	                                   0, 0, 0, 0, tty_stderrwrite, 0, 0, 0);
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
                         fs_write_file_fpointer writefile,
                         fs_open_dir_fpointer   opendir,
                         fs_read_dir_fpointer readdir, fs_make_node makenode,
                         fs_creat_fpointer creat, fs_update_fpointer update)
{
	g_current_fs->name = name;
	g_current_fs->type = type;

	g_current_fs->block_read  = readfile;
	g_current_fs->block_write = writefile;
	g_current_fs->dir_open    = opendir;
	g_current_fs->dir_read    = readdir;
	g_current_fs->create_node = creat;
	g_current_fs->update_node = update;
	g_current_fs->fs_makenode = makenode;
}

ssize_t fs_read(vfs_node_t *node, int seek, void *_buf, size_t amount)
{
	/* @XXX: I am VERY unsure about this piece of code here */
	char *buf = _buf;

	ssize_t      ret       = 0;
	unsigned int blocksize = node->fs_info->block_size;

	unsigned int blockiter = seek / blocksize; /* the startblock*/

	unsigned int s_rest = seek % blocksize;
	unsigned int e_rest = (amount + s_rest) % blocksize;

	if (s_rest && blocksize - s_rest > amount)
		e_rest = 0;

	int blkcnt = (amount - e_rest) / blocksize;

	if (s_rest)
	{
		char *tmp = kmalloc(blocksize);
		node->fs_info->block_read(node->offset, blockiter++, tmp, 1,
		                          node->fs_info);

		size_t size = blocksize - s_rest;
		if (amount < size)
			size = amount;

		memcpy(buf, tmp + s_rest, size);
		ret += size;
		buf += size;
		kfree(tmp);
	}
	if (blkcnt)
	{
		int blks = node->fs_info->block_read(node->offset, blockiter, buf,
		                                     blkcnt, node->fs_info);
		ret += blks * blocksize;
		buf += blks * blocksize;
		blockiter += blkcnt;

		if (blks != blkcnt)
			return ret;
	}
	if (e_rest)
	{
		char *tmp = kmalloc(blocksize);
		node->fs_info->block_read(node->offset, blockiter, tmp, 1,
		                          node->fs_info);
		memcpy(buf, tmp, e_rest);
		kfree(tmp);
		ret += e_rest;
	}

	return ret;
}

ssize_t fs_write(vfs_node_t *node, int seek, const void *_buf, size_t amount)
{
	const char *buf = _buf;

	ssize_t      ret       = 0;
	unsigned int blocksize = node->fs_info->block_size;

	filesystem_t *fs_info = node->fs_info;

	unsigned int blockiter = seek / blocksize; /* the startblock*/

	unsigned int s_rest = seek % blocksize;
	unsigned int e_rest = (amount + s_rest) % blocksize;

	if (s_rest && blocksize - s_rest > amount)
		e_rest = 0;

	int blkcnt = (amount - e_rest) / blocksize;

	if (s_rest)
	{
		char *tmp = kmalloc(blocksize);
		fs_info->block_read(node->offset, blockiter, tmp, 1, fs_info);

		size_t size = blocksize - s_rest;
		if (amount < size)
		{
			e_rest = 0;
			size   = amount;
		}

		memcpy(tmp + s_rest, buf, size);

		fs_info->block_write(node->offset, blockiter++, tmp, 1, fs_info);
		ret += size;
		buf += size;
		kfree(tmp);

		if (blkcnt)
			blkcnt--;
	}
	if (blkcnt)
	{
		int blks =
			fs_info->block_write(node->offset, blockiter, buf, blkcnt, fs_info);

		blockiter += blks;
		buf += blks * blocksize;
		ret += blks * blocksize;

		if (blks != blkcnt)
			return ret;
	}
	if (e_rest)
	{
		char *tmp = kmalloc(blocksize);
		fs_info->block_read(node->offset, blockiter, tmp, 1, fs_info);
		memcpy(tmp, buf, e_rest);

		fs_info->block_write(node->offset, blockiter, tmp, 1, fs_info);

		kfree(tmp);
		ret += e_rest;
	}

	node->filelength = ret + seek;

	fs_info->update_node(node);
	return ret;
}

int fs_creat(vfs_node_t *node, char *path, flags_t flags)
{
	return node->fs_info->create_node(node, path, flags);
}