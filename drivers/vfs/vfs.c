#include <drivers/vfs/vfs.h>
#include <drivers/vfs/vfs_node.h>
#include <drivers/ramdisk/ramdisk.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>
#include <lib/string/string.h>

vfs_node_t *g_vfs_root;
uint32_t g_nodecount;


/**
 * @brief      VFS read file function
 *
 * @param      node    The vfs node
 * @param[in]  offset  The offset (or inode) of the file
 * @param      buffer  The buffer to write to
 * @param[in]  size    The amount of bytes to read
 *
 * @return     amount of bytes read
 */
int vfs_read(vfs_node_t *node, uint32_t offset, uint8_t *buffer, size_t size)
{
	if (node->read != 0) {
		return node->read(node, offset, buffer, size);
	} else {
		return 0;
	}
}

/**
 * @brief      VFS write file function
 *
 * @param      node    The vfs node
 * @param[in]  offset  The offset (or inode) of the file
 * @param      buffer  The buffer to read from
 * @param[in]  size    The amount of bytes to write
 *
 * @return     amount of bytes written
 */
int vfs_write(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	if (node->write != 0) {
		return node->write(node, offset, buffer, size);
	} else {
		return 0;
	}
}

/**
 * @brief      Opens a vfs node
 *
 * @param      node   The node to open
 * @param[in]  flags  The flags
 * @param[in]  mode   The mode to open the node in
 *
 * @return     success or failure (0/-1)
 */
int vfs_open(vfs_node_t *node, int flags, int mode)
{
	if (node->open != 0) {
		return node->open(node, flags, mode);
	} else {
		return -1;
	}
}

/**
 * @brief      Closes a vfs node
 *
 * @param      node  The node to close
 *
 * @return     success or failure (0/-1)
 */
int vfs_close(vfs_node_t *node)
{
	if (node->close != 0) {
			return node->close(node);
		} else {
			return -1;
		}
}

/**
 * @brief      Reads the contents from a directory stream
 *
 * @param      node       The node pointing to the dir
 * @param      dirstream  The directory stream
 *
 * @return     A dirent struct
 */
struct dirent *vfs_readdir(vfs_node_t *node, DIR *dirstream)
{
	if (node->type == VFS_DIRECTORY && node->readdir != 0) {
		return (struct dirent*) node->readdir(dirstream);
	} else {
		return 0;
	}
}


#if 0
/**
 * @brief      Sets up a virtual filesystem node
 *
 * @param[in]  type        The type of file
 * @param[in]  filelength  The filelength
 * @param[in]  offset      The file offset
 * @param[in]  open        The open function pointer
 * @param[in]  close       The close function pointer
 * @param[in]  read        The read function pointer
 * @param[in]  write       The write function pointer
 * @param[in]  readdir     The readdir function pointer
 * @param[in]  finddir     The finddir function pointer
 *
 * @return     { description_of_the_return_value }
 */
vfs_node_t* vfs_setupnode(uint8_t type, uint32_t filelength, uint32_t offset, open_fpointer open, close_fpointer close, 
						  read_fpointer read, write_fpointer write, read_dir_fpointer readdir, find_dir_fpointer finddir)
{
	vfs_node_t* n = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	n->id = g_nodecount++;
	n->type = type;
	n->filelength = filelength;
	n->offset = offset;
	n->open = open;
	n->close = close;
	n->read = read;
	n->write = write;
	n->readdir = readdir;
	n->finddir = finddir;
	return n;
}

#endif

/**
 * @brief      A stupid strcmp function
 * 
 * @todo       should create working strcmp function in libc
 * 
 * @param      a     
 * @param      b     
 *
 * @return     0 when equal
 */
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
	int i = 1;
	int last = i;
	char c = path[i];
	vfs_node_t *node = g_vfs_root;
	while (1) {
		if ((c == '\0' || c == '/') && (i-last) != 0) {
			memcpy(buf, path+last, i-last);
			buf[i-last+1] = '\0';
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
