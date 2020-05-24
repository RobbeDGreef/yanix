#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <drivers/disk.h>
#include <drivers/ramdisk.h>
#include <fs/filedescriptor.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>
#include <libk/string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <kernel.h>
#include <unistd.h>
#include <libk/math.h>
#include <debug.h>

vfs_node_t	   *g_vfs_root  = NULL;
unsigned long 	g_nodecount = 0;

static ssize_t _vfs_read(struct file_descriptor* fd_struct, void *buf, size_t amount)
{
	ssize_t ret = 0;
	vfs_node_t *node = fd_struct->node;
	if (node->read != 0)
	{
		ret = node->read(node, fd_struct->seek, buf, amount);
		fd_struct->seek += ret;
		return ret;
	}
	else if (node->fs_info->block_read)
	{
		ret = fs_read(node, fd_struct->seek, buf, amount);
		fd_struct->seek += ret;
		return ret;
	}
	else
	{
		errno = EUNATCH;
		return -1;
	}
}

/**
 * @brief      Check wheter the VFS is initialsed
 *
 * @return     True if vfs initialised
 */
int vfs_check_if_initialised()
{
	if (g_vfs_root == 0)
		return 0;

	return 1;
}

/**
 * @brief      Reads from filedescriptor
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer to write to
 * @param[in]  amount  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read_fd(int fd, void *buf, size_t amount)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);

	if (fd_struct == 0)
		return -1;

	return _vfs_read(fd_struct, buf, amount);
}

/**
 * @brief      Reads from a file structure
 *
 * @param      file    The opened file struct to read from
 * @param      buf     The buffer to write to
 * @param[in]  amount  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read(struct file *file, void *buf, size_t amount)
{
	return _vfs_read(get_filedescriptor(file->filedescriptor), buf, amount);
}


/**
 * @brief      Writes to a vfs_node
 *
 * @param      node    The node to write to
 * @param      buf     The buffer to read from
 * @param[in]  amount  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
static ssize_t _vfs_write(struct file_descriptor *fd_struct, const void *buf, size_t amount)
{
	ssize_t ret = 0;
	vfs_node_t *node = fd_struct->node;
	if (node->write != 0)
	{
		ret = node->write(node, fd_struct->seek, buf, amount);
		fd_struct->seek += ret;
		return ret;
	}
	else if (node->fs_info->block_write)
	{
		ret = fs_write(node, fd_struct->seek, buf, amount);
		fd_struct->seek += ret;
		return ret;
	}
	else
	{
		errno = EUNATCH;
		return -1;
	}
}

/**
 * @brief      Writes to filedescriptor
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer to read from
 * @param[in]  amount  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write_fd(int fd, const void *buf, size_t amount)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);
	
	if (fd_struct  == 0)
		return -1;

	return _vfs_write(fd_struct, buf, amount);
}


/**
 * @brief      Writes to a file structure
 *
 * @param      file    The opened file struct to write to
 * @param      buf     The buffer to read from
 * @param[in]  amount  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write(struct file *file, const void *buf, size_t amount)
{
	return _vfs_write(get_filedescriptor(file->filedescriptor), buf, amount);
}

static int _get_type(int mode)
{
	if (S_ISDIR(mode)) {
		return VFS_DIRECTORY;
	} else if (S_ISCHR(mode)) {
		return VFS_CHARDEVICE;
	} else if (S_ISLNK(mode)) {
		return VFS_SYMLINK;
	} else if (S_ISFIFO(mode)) {
		return VFS_PIPE;
	} else if (S_ISBLK(mode)) {
		return VFS_BLOCKDEVICE;
	} else if (S_ISLNK(mode)) {
		return VFS_SYMLINK;
	} else if (S_ISSOCK(mode)) {
		return VFS_SOCKET;
	} else {
		return VFS_FILE;
	}
}

static vfs_node_t *_find_dir_path(const char *nodepath)
{
	char *path = kmalloc(strlen(nodepath) + 1);
	memcpy(path, nodepath, strlen(nodepath) + 1);

	/* Strip off trailing / */
	if (path[strlen(path) - 1] == '/')
		path[strlen(path) - 1] = '\0';

	char *cutoff = strchr_r(path, '/');
	*cutoff = '\0';

	vfs_node_t *dir = vfs_find_path(path);
	kfree(path);

	return dir;
}

static vfs_node_t *_create_node(const char *nodepath, int flags, mode_t mode)
{
	/* @todo: see creat(); (we need to take into account flags parameter) */
	(void) (flags);

	printk("Trying to create: %s\n", nodepath);

	vfs_node_t *dir = _find_dir_path(nodepath);
	if (dir == NULL)
		return NULL;

	vfs_node_t *node = kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));

	node->type = _get_type(mode);
	// @todo: permissions, uid, gid
	node->id = g_nodecount++;
	node->fs_info = dir->fs_info;
	
	/* Connect the node into the directory */
	node->parent = dir;

	vfs_node_t *tmp = dir->dirlist;
	while (tmp->nextnode)
		tmp = tmp->nextnode;
	
	tmp->nextnode = node;
	return node;
}

vfs_node_t *_vfs_open(const char *path, int flags, int mode)
{
	vfs_node_t *node = vfs_find_path(path);
	int node_created = 0;

	/* if node was not found check if O_CREAT flag is set */
	if (!node && flags & O_CREAT)
	{
		/*create the node */
		node = _create_node(path, flags, mode);
		node_created = 1;

		if (!node)
			return NULL;

		printk("node created\n");
		
		if (node->creat || (node->fs_info && node->fs_info->create_node))
		{
			int i;
			for (i = strlen(path); i >= 0; --i)
			{
				if (path[i] == '/' && (unsigned) i != strlen(path))
					break;
			}

			char *newpath = kmalloc(strlen(path) - i);
			memcpy(newpath, path + i + 1, strlen(path) - i);

			if (newpath[strlen(newpath)] == '/')
				newpath[strlen(newpath)] = '\0';
			
			int ret;
			if (node->creat)
				ret = node->offset = node->creat(node, newpath, flags);
			else
				ret = fs_creat(node, newpath, flags);

			if (ret == -1)
			{
				kfree(node);
				node = NULL;
			}

			kfree(newpath);
		}
	}
	else if (!node)
	{
		printk("Not found %s\n", path);
		return NULL;
	}
 
	if (node->open && node->open(node, !node_created, flags, mode) == -1) 
		return NULL;

	return node;
}

int _vfs_stat(vfs_node_t *node, mode_t mode, struct stat *statbuf)
{	
	if (!node)
		return -1;

	/* See man stat(2) for these values */
	statbuf->st_dev = 0;						/* @todo: What is the st_dev ? */
	statbuf->st_ino = node->id;
	statbuf->st_mode = (mode_t) node->type | mode;
	statbuf->st_nlink = node->nlink;
	statbuf->st_uid	= node->uid;
	statbuf->st_gid = node->gid;
	statbuf->st_rdev = 0;						/* @todo: rdev number in vfs nodes ? see man stat(2) */
	statbuf->st_size = node->filelength;
	statbuf->st_blksize = node->fs_info->block_size;
	statbuf->st_blocks = roundup(node->filelength, 512);
	/* @todo: access times in stat struct */
	return 0;
}


int vfs_fstat(int fd, struct stat *statbuf)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);
	
	if (fd_struct)
		return _vfs_stat(fd_struct->node, fd_struct->mode, statbuf);

	return -1;
}

int vfs_stat(const char *pathname, struct stat *statbuf)
{
	vfs_node_t *node = vfs_find_path(pathname);
	struct file_descriptor *fd_struct = get_filedescriptor_from_node(node);
	
	if (fd_struct)
		return _vfs_stat(node, fd_struct->mode, statbuf);

	return _vfs_stat(node, 0, statbuf);
}

off_t vfs_lseek(int fd, off_t offset, int whence)
{
	struct file_descriptor *fd_struct = get_filedescriptor(fd);

	if (whence == SEEK_CUR)
		offset += fd_struct->seek;

	else if (whence == SEEK_END)
	{
		struct stat statbuf;
		if (vfs_fstat(fd, &statbuf) == -1)
			return -1;

		offset += statbuf.st_size;
	}

	fd_struct->seek = offset;

	return offset;
}

DIR *_vfs_opendir(vfs_node_t *node)
{
	if (!node)
		return 0;

	if (node->opendir)
	{
		return node->opendir(node);
	}
	else if (node->fs_info && node->fs_info->dir_open)
	{
		return node->fs_info->dir_open(node->offset, node->fs_info);
	}

	return 0;
}
 #include <libk/string.h>
/**
 * @brief      Open a file descriptor
 *
 * @param[in]  path   The path
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode
 *
 * @return     Filedescriptor on success
 */
int vfs_open_fd(const char* path, int flags, int mode)
{
	vfs_node_t *node;
	if (strcmp(path, "/testout.asm") == 0)
		node = get_filedescriptor(1)->node; 
	else
	 	node = _vfs_open(path, flags, mode);
	

	if (!node)
		return -errno;

	int fd = register_filedescriptor(node, mode);

	if (flags & O_DIRECTORY)
	{
		DIR *d = _vfs_opendir(node);
		vfs_lseek(fd, (unsigned int) d, SEEK_SET);
	}

	return fd;
}


/**
 * @brief      Opens a vfs node
 *
 * @param[in]  path   The filepath
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode to open the node in
 *
 * @return     file structure or failture (0)
 */
struct file *vfs_open(const char *path, int flags, int mode)
{
	vfs_node_t *node = _vfs_open(path, flags, mode);
	
	if (node == 0)
		return 0;
	
	int fd = register_filedescriptor(node, mode);

	if (fd == -1) {
		return 0;
	}

	struct file *file = (struct file*) kmalloc(sizeof(file));
	if (file == 0) {
		return 0;
	}
	file->vfs_node = node;
	file->filedescriptor = fd;
	file->filesize = node->filelength;
	return file;
}

char *vfs_get_name(const char*path)
{
	int pathlen = strlen(path);
	char *tmp = kmalloc(pathlen);
	memcpy(tmp, path, pathlen);

	if (path[pathlen-1] == '/')
	{
		tmp[pathlen-1] = '\0';
		pathlen--;
	}
	

	for (int i = pathlen; i != 0; i--)
	{
		if (tmp[i] == '/')
		{
			return &tmp[i+1];
		}
	}
	kfree(tmp);

	return 0;
}

int vfs_link_node_vfs(const char *path, vfs_node_t *node)
{
	int pathlen = strlen(path);
	char *tmp = kmalloc(pathlen);
	memcpy(tmp, path, pathlen);

	if (path[pathlen-1] == '/')
	{
		tmp[pathlen-1] = '\0';
		pathlen--;
	}
	
	vfs_node_t *dir;

	for (int i = pathlen; i != 0; i--)
	{
		if (tmp[i] == '/')
		{
			tmp[i] = '\0';
			dir = vfs_find_path(tmp);
		}
	}

	if (dir == 0)
		return -1;

	node->parent = dir;

	vfs_node_t *ntmp = dir->dirlist;
	
	while (ntmp->nextnode != 0)
		ntmp = ntmp->nextnode;
	
	ntmp->nextnode = node;

	kfree(tmp);

	return 0;
}

/**
 * @brief      Creates file and opens it
 *
 * @param[in]  path  The path
 * @param[in]  mode  The mode
 *
 * @return     file descriptor
 */
int vfs_creat(const char *path, int mode)
{
	return vfs_open_fd(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

/**
 * @brief      Closes a vfs node
 *
 * @param      file  The file structure of the opened file
 *
 * @return     success or failure (0/-1)
 */
int vfs_close(struct file *file)
{
	close_filedescriptor(file->filedescriptor);
	int ret = 0;
	if (file->vfs_node->close != 0) {
		ret = file->vfs_node->close(file->vfs_node);
	}
	if (kfree(file) == -1) {
		return -1;
	}

	return ret;
}

/**
 * @brief      Closes a filedescriptor
 *
 * @param[in]  fd    The filedescriptor
 *
 * @return     success
 */
int vfs_close_fd(int fd)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	
	if (node == 0)
		return -1;
	
	else if (node->close != 0)
		node->close(node);
	
	return close_filedescriptor(fd);
}


/**
 * @brief      Reads the contents from a directory stream
 *
 * @param      dirstream  The directory stream
 *
 * @return     A dirent struct
 */
struct dirent *vfs_readdir(DIR *dirstream)
{
	if (dirstream && dirstream->fs_info && dirstream->fs_info->dir_read)
		return dirstream->fs_info->dir_read(dirstream);


	return 0;
}

/**
 * @brief      Opens a directory stream
 *
 * @param[in]  filepath  The filepath
 *
 * @return     The directory stream or an error
 */
DIR *vfs_opendir(const char *filepath)
{
	vfs_node_t *node = vfs_find_path(filepath);

	return _vfs_opendir(node);
}

/**
 * @brief      Function to find specific node by name in the vfs system
 *
 * @param      node  The node
 * @param      name  The name
 *
 * @return     { description_of_the_return_value }
 */
vfs_node_t *_vfs_path_find(vfs_node_t *node, char *name)
{
	#if 0
	DIR *dirp = node->opendir(node);

	if (dirp == 0) {
		return 0;
	}

	struct dirent *dir;
	uint32_t searchinode = 0;
	while ((dir = node->readdir(dirp)) != 0) {
		if (_strcmpI(name, dir->d_name) == 0) {
			// found right inode
			searchinode = dir->d_ino;
			break;
		} 
	}
	if (searchinode == 0) {
		return 0;
	}

	// find the right vfs_node in the linked list
	vfs_node_t *tmp = node->dirlist;
	while (tmp != 0) {
		if (tmp->offset == searchinode) {
			return tmp;
		}
		tmp = tmp->nextnode;
	}
	return 0;

	#endif

	vfs_node_t *tmp = node->dirlist;

	while (tmp)
	{
		if (!strcmp(name, tmp->name))
			return tmp;
		tmp = tmp->nextnode;
	}

	errno = ENOENT;
	return 0;
}


/**
 * @brief      Finds the inode pointing to a specific path
 *
 * @param      path  The path 
 *
 * @return     The inode to look for
 */
vfs_node_t *vfs_find_path(const char *path)
{
	#if 0
	char *buf = (char*) kmalloc(strlen(path)+1);
	memset(buf, 0, strlen(path)+1);
	int i = 1;
	int last = i;
	char c = path[i];
	vfs_node_t *node = g_vfs_root;
	while (1) {
		if ((c == '\0' || c == '/') && (i-last) != 0) {
			memcpy(buf, path+last, i-last);
			buf[i-last] = '\0';
			last = i+1;
			node = _vfs_path_find(node, buf);
			if (node == 0) {
				break;
			}
			if (c == '\0') {
				break;
			}
		} else if (c == '\0' || c == '/') {
			break;
		}
		i++;
		c = path[i];
	}
	kfree(buf);
	if (node == 0) {
		errno = ENOENT;
	}
	return node;
	#endif

	/**
	 * 
	 * It works pretty simple
	 * 
	 * - if the path starts with a / -> its an absolute path
	 * - if the path doesn't start with a / -> its a path relative of the current working directory
	 * 
	 */

	if (path[0] != '/')
	{
		printk(KERN_WARNING "Kernel doesn't support relative filepaths yet, thus '%s' was not valid", path);
	}

	size_t pathlength 	= strlen(path) + 1;
	char  *buffer 		= kmalloc(pathlength);
	size_t bufferiter 	= 0;
	vfs_node_t *node 	= g_vfs_root;

	if (!node)
	{
		printk(KERN_WARNING "vfs not initialized ?\n");
		return NULL;
	}

	memset(buffer, 0, pathlength);

	/* iter starts at 1 because 0 is the first '/' character */
	for (size_t i = 1; i < pathlength; i++)
	{
		if (path[i] == '/' || path[i] == '\0')
		{
			/* @fixme: Whenever we want to find '/' we can't because it looks in the subdirectory of that node anyway we need to fix that */
			buffer[bufferiter++] = '\0';
			node = _vfs_path_find(node, buffer);

			if (!node)
			{
				kfree(buffer);
				return 0;
			}

			if (path[i] == '\0')
			{
				kfree(buffer);
				return node;
			}
			else
			{
				memset(buffer, 0, bufferiter+1);
				bufferiter = 0;
			}
		}
		else
			buffer[bufferiter++] = path[i];
		
	}

	errno = ENOENT;
	return 0;
}

vfs_node_t *vfs_setupnode(char *name, uint8_t type, uint16_t permissions, uid_t uid, gid_t gid, size_t size, offset_t offset,
						  open_fpointer open, close_fpointer close, creat_fpointer creat, read_fpointer read, write_fpointer write,
						  open_dir_fpointer opendir, read_dir_fpointer readdir, filesystem_t *fs_info)
{
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));
	node->name = name;
	node->type = type;
	node->permissions = permissions;
	node->uid = uid;
	node->gid = gid;
	node->id = g_nodecount++;
	node->filelength = size;
	node->fs_info = fs_info;
	node->offset = offset;
	node->open = open;
	node->close = close;
	node->creat = creat;
	node->read = read;
	node->write = write;
	node->opendir = opendir;
	node->readdir = readdir;
	// @todo: the rest
	
	return node;
}

/**
 * @brief      Recursive function that loops over the filesystem and allocates all the nodes
 *
 * @param[in]  start      From what inode this loop should start
 * @param[in]  rootnode   Whether this is the top of the filesystem or not
 * @param      startnode  The node to start the linked list from
 * @param      fs_info    The file system information
 */
void loop_over_filesystem(uint32_t start, int rootnode, vfs_node_t *startnode, filesystem_t *fs_info)
{	
	/* If it's the root inode, add it as the first in the list */
	if (rootnode) 
	{
		g_vfs_root = (vfs_node_t*) fs_info->fs_makenode(start, "", g_nodecount++, fs_info);
		g_vfs_root->parent = g_vfs_root;
		startnode = g_vfs_root;
	}
	
	/* Opens the given inode directory */
	DIR *dirp = fs_info->dir_open(start, fs_info);
	if (dirp == 0)
	{
		return;
	}
	int first = 1;
	struct dirent *dir;
	vfs_node_t *prevnode = startnode;
	vfs_node_t *node;

	/* Read all the entries in the opened inode */
	while ((dir = fs_info->dir_read(dirp)) != 0)
	{
		/* The files . and .. shouldn't be added in the vfs */
		//if (_strcmpI(dir->d_name, ".") != 0 && _strcmpI("..", dir->d_name) != 0)
		//{
			/* Make name string */
			char *name = kmalloc(strlen(dir->d_name)+1);
			memcpy(name, &dir->d_name, strlen(dir->d_name));
			name[strlen(dir->d_name)] = '\0';

			/* Create a new inode */
			//printk("Inode %u name %s id %i\n", dir->d_ino, name, g_nodecount, fs_info);
			node = (vfs_node_t*) fs_info->fs_makenode(dir->d_ino, name, g_nodecount++, fs_info);

			if (node == 0) 
			{
				continue;
			}

			node->parent = prevnode;

			if (first)
			{
				prevnode->dirlist = node;
				first = 0;
			} else {
				prevnode->nextnode = node;
			}

			if (node->type == VFS_DIRECTORY && (strcmp(dir->d_name, ".") && strcmp("..", dir->d_name))) {
				/* Recursive function */
				loop_over_filesystem(node->offset, 0, node, fs_info);
			}
			prevnode = node;
		//}

	}

	fs_info->dir_close(dirp);
}

int check_vfs_initialised()
{
	return g_vfs_root ? 1:0;
}

/* @TODO: this is not how this system should work at all (see note below) */
extern disk_t *disk_list;
#include <debug.h>
/**
 * @brief      Initialises the virtual filesystem
 */
int init_vfs()
{
	/* @TODO: this is totally not how the vfs should be initialised */
	g_nodecount = 0;
	/* @todo: hard coded entry of fs is REALY BAD here */
	g_current_fs = init_ext2_filesystem("EXT2FS", 1048576, disk_list);
	
	if (g_current_fs == 0)
	{
		return -1;
	}

	g_vfs_root = 0;
	//init_vfs_lookuptable(g_vfs_root);

	loop_over_filesystem(2, 1,  g_vfs_root, g_current_fs);
	return 0;
}
