/**
 * drivers/vfs/vfs.c
 * 
 * Author: Robbe De Greef
 * Date:   31 may 2019
 * 
 * Version 3.0
 */

#include <drivers/vfs/vfs.h>
#include <drivers/vfs/vfs_node.h>
#include <drivers/ramdisk/ramdisk.h>
#include <fs/filedescriptor.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>
#include  <libk/string/string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


vfs_node_t *g_vfs_root;
uint32_t g_nodecount;

/**
 * @brief      Reads from a vfs_node
 *
 * @param      node    The node to read from
 * @param      buf     The buffer to write to
 * @param[in]  amount  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
static ssize_t _vfs_read(vfs_node_t* node, void *buf, size_t amount)
{
	if (node->read != 0) {
		return node->read(node, node->offset, buf, amount);
	} else {
		errno = EUNATCH;
		return -1;
	}
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
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == 0) {
		return -1;
	}
	return _vfs_read(node, buf, amount);
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
	return _vfs_read(file->vfs_node, buf, amount);
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
static ssize_t _vfs_write(vfs_node_t *node, const void *buf, size_t amount)
{
	if (node->write != 0) {
		return node->write(node, node->offset, buf, amount);
	} else {
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
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == 0) {
		return -1;
	}

	return _vfs_write(node, buf, amount);
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
	return _vfs_write(file->vfs_node, buf, amount);
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

static vfs_node_t *_create_node(const char *nodepath, int flags, int mode)
{
	//@todo: see creat(); (we need to take into account flags parameter)
	(void) (flags);

	// just so we can edit the path easier
	char *path = (char*) kmalloc(sizeof(char) * (strlen(nodepath)+1));
	memcpy(path, nodepath, sizeof(char) * (strlen(nodepath)+1));

	// if the path ends on '/' snip it of
	if (path[strlen(path)-1] == '/') {
		path[strlen(path)-1] = '\0';
	}

	vfs_node_t *dir = 0;
	// snip of path at last '/' in other words get directory of node
	for (int i = strlen(path); i >= 0; i--) {
		// if the path ends on / and this is not the first char in the path snip it of 
		if (path[i] == '/') {
			if (i == 0) {
				path[i+1] = '\0';
			} else {
				path[i] = '\0';
			}

			// dir path created, now try to find the directory
			dir = vfs_find_path(path);

			// otherwise continue
			break;
		}
	}

	// if directory doesn't exist return with error
	if (dir == 0) {
		errno = ENOENT;
		return 0;
	}

	// created new node in memory
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));

	// set all the values

	node->type = _get_type(mode);
	// @todo: permissions, uid, gid
	node->id = g_nodecount++;
	node->fs_info = dir->fs_info;
	

	if (node->type == VFS_FILE || node->type == VFS_DIRECTORY) {
		node->open = dir->open;
		node->close = dir->close;
		node->read = dir->read;
		node->write = dir->write;
		node->creat = dir->creat;
		node->opendir = dir->opendir;
		node->closedir = dir->closedir;
		node->readdir = dir->readdir;
	}
	
	// now connect the node into the directory
	node->parent = dir;
	vfs_node_t *tmp = dir->dirlist;
	while (tmp->nextnode != 0) {
		tmp = tmp->nextnode;
	}
	tmp->nextnode = node;
	kfree(path);

	return node;
}

vfs_node_t *_vfs_open(const char *path, int flags, int mode)
{
	// find a node
	vfs_node_t *node = vfs_find_path(path);
	int node_already_exists = 1;
	// if node was not found check if O_CREAT flag set (and node needs to be created)
	if (node == 0 && (flags & O_CREAT) != 0) {
		// create the node
		node = _create_node(path, flags, mode);
		node_already_exists = 0;

		// if node was not created return with error
		if (node == 0) {
			return 0;
		}
		if (node->creat != 0) {
			int i;
			for (i = strlen(path); i >= 0; i--) {
				if (path[i] == '/' && (unsigned int) i != strlen(path)){
					break;
				}
			}
			char *newpath = (char*) kmalloc(sizeof(char) * (strlen(path)-(unsigned int)i));
			memcpy(newpath, &path[i+1], strlen(path)-i);
			if (newpath[strlen(newpath)] == '/') {
				newpath[strlen(newpath)] = '\0';
			}
			node->offset = node->creat(node, newpath, mode);
			kfree(newpath);
		}

		// node created so continue on
	} else if (node == 0) {
		// not found and shouldn't create so return with error
		return 0;
	}
	// if node has open function, run open function and if return value == -1 return with error
	if (node->open != 0) {
		if (node->open(node, node_already_exists, flags, mode) == -1) {
			return 0;
		}
	}

	return node;
}


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
	vfs_node_t *node = _vfs_open(path, flags, mode);
	if (node == 0) {
		return -1;
	}
	return register_filedescriptor(node, mode);
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
	if (node == 0) {
		return 0;
	}
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
	if (node == 0) {
		return -1;
	} else if (node->close != 0) {
		node->close(node);
	}
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
	return dirstream->fs_info->dir_read(dirstream);
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
	if (node == 0 && node->opendir != 0) {
		return 0;
	}
	return node->opendir(node);
}


/**
 * @brief      A wrong strcmp function
 * 
 * @todo       should create working strcmp function in libc
 * 
 * @param      a     
 * @param      b     
 *
 * @return     0 when equal
**/
int _strcmpI(char *a, char *b)
{
	if (strlen(a) == strlen(b)) {
		return memcmp(a, b, strlen(a));
	} else {
		return -1;
	}
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
}

vfs_node_t *vfs_setupnode(uint8_t type, uint16_t permissions, uid_t uid, gid_t gid, size_t size, offset_t offset,
						  open_fpointer open, close_fpointer close, creat_fpointer creat, read_fpointer read, write_fpointer write,
						  open_dir_fpointer opendir, read_dir_fpointer readdir, filesystem_t *fs_info)
{
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));
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
	if (rootnode) {
		g_vfs_root = (vfs_node_t*) fs_info->fs_makenode(start, g_nodecount++, fs_info);
		startnode = g_vfs_root;
	}
	
	DIR *dirp = fs_info->dir_open(start, fs_info);
	if (dirp == 0){
		return;
	}
	int first = 1;
	struct dirent *dir;
	vfs_node_t *prevnode = startnode;
	vfs_node_t *node;
	while ((dir = fs_info->dir_read(dirp)) != 0){
		if (_strcmpI(dir->d_name, ".") != 0 && _strcmpI("..", dir->d_name) != 0){
			node = (vfs_node_t*) fs_info->fs_makenode(dir->d_ino, g_nodecount++, fs_info);
			if (node == 0) {
				continue;
			}
			node->parent = prevnode;
			if (first){
				prevnode->dirlist = node;
				first = 0;
			} else {

				prevnode->nextnode = node;
			}
			if (node->type == VFS_DIRECTORY) {
				loop_over_filesystem(node->offset, 0, node, fs_info);
			}
			prevnode = node;
		}
	}

	fs_info->dir_close(dirp);
}


/**
 * @brief      Initialises the virtual filesystem
 */
void init_vfs()
{
	g_nodecount = 0;
	g_current_fs = ext2_initialize_filesystem("EXT2FS", &ramdisk_read, &ramdisk_write);
	g_vfs_root = 0;
	//init_vfs_lookuptable(g_vfs_root);
	loop_over_filesystem(2, 1, g_vfs_root, g_current_fs);

}
