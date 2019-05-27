#include <drivers/vfs/vfs.h>
#include <drivers/vfs/vfs_node.h>
#include <drivers/ramdisk/ramdisk.h>
#include <fs/filedescriptor.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>
#include <lib/string/string.h>
#include <errno.h>

vfs_node_t *g_vfs_root;
uint32_t g_nodecount;


/**
 * @brief      VFS read file function
 *
 * @param      file  The opened file struct to read from
 * @param      buf   The buffer to write to
 * @param[in]  size  The amount of bytes to read
 *
 * @return     amount of bytes read
 */
ssize_t vfs_read(struct file *file, void *buf, size_t size)
{
	if (file->vfs_node->read != 0) {
		return file->vfs_node->read(file->vfs_node, file->vfs_node->offset, buf, size);
	} else {
		// read function not attached so -> protcol driver not attached error returned
		errno = EUNATCH;
		return -1;
	}
}

/**
 * @brief      VFS write file function
 *
 * @param      file  The opened file structure
 * @param      buf   The buffer to read from
 * @param[in]  size  The amount of bytes to write
 *
 * @return     amount of bytes written
 */
ssize_t vfs_write(struct file *file, const void *buf, size_t size)
{
	if (file->vfs_node->write != 0) {
		return file->vfs_node->write(file->vfs_node, file->vfs_node->offset, buf, size);
	} else {
		// write function not attached so -> protcol driver not attached error returned
		errno = EUNATCH;
		return -1;
	}
}

/**
 * @brief      Opens a vfs node
 *
 * @param[in]  filepath  The filepath
 * @param[in]  flags     The flags
 * @param[in]  mode      The mode to open the node in
 *
 * @return     file structure or failture (0)
 */
struct file *vfs_open(const char *filepath, int flags, int mode)
{
	vfs_node_t* node = vfs_find_path(filepath);
	if (node == 0) {
		return 0;
	}
	int fd = register_filedescriptor(node, mode);

	if (node->open != 0) {
		node->open(node, flags, mode);
	}
	struct file *file = (struct file*) kmalloc(sizeof(file));
	if (file == 0) {
		return 0;
	}
	file->vfs_node = node;
	file->filedescriptor = fd;
	file->length = node->filelength;
	return file;
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
	if (node == 0) {
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


/**
 * @brief      Reads from filepointer (for syscalls)
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer
 * @param[in]  amount  The amount
 *
 * @return     amount of bytes read on success
 */
ssize_t vfs_read_raw(int fd, void *buf, size_t amount)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (fd == 0) {
		return -1;
	}
	if (node->read != 0) {
		return node->read(node, node->offset, buf, amount);
	} else {
		errno = ENOSYS;
		return -1;
	}
}

/**
 * @brief      Writes to filepointer (for syscalls)
 *
 * @param[in]  fd      The filedescriptor
 * @param      buf     The buffer
 * @param[in]  amount  The amount
 *
 * @return     amount of bytes written on success
 */
ssize_t vfs_write_raw(int fd, const void *buf, size_t amount)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (fd == 0) {
		return -1;
	}
	if (node->read != 0) {
		return node->write(node, node->offset, buf, amount);
	} else {
		errno = ENOSYS;
		return -1;
	}
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
int vfs_open_raw(const char* path, int flags, int mode)
{
	vfs_node_t *node = vfs_find_path(path);
	if (node == 0) {
		return -1;
	} else if (node->open != 0) {
		node->open(node, flags, mode);
	}
	return register_filedescriptor(node, mode);
}

/**
 * @brief      Closes a filedescriptor
 *
 * @param[in]  fd    The filedescriptor
 *
 * @return     success
 */
int vfs_close_raw(int fd)
{
	vfs_node_t *node = get_filedescriptor_node(fd);
	if (node == 0) {
		errno = ENODEV;
		return -1;
	} else if (node->close != 0) {
		node->close(node);
	}
	return close_filedescriptor(fd);
}