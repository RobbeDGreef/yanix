#include <drivers/ramdisk/ramdisk.h>
#include <drivers/video/videoText.h>
#include <lib/string/string.h>
#include <lib/math/math.h>
#include <drivers/vfs/vfs.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>

#include <sys/types.h>

/**
 * @brief      Gets a pointer to the superblock.
 *
 * @param[in]  read  The read function pointer
 *
 * @return     Pointer to the superblock.
 */
ext2_superblock_t *_ext2_get_superblock(fs_read_fpointer read){
	// superblock is always located at 1024 
	//return (ext2_superblock_t*) ramdisk_get(EXT2_SUPERBLOCK_START_LOCATION);
	ext2_superblock_t *ret = (ext2_superblock_t*) kmalloc(sizeof(ext2_superblock_t));
	read(EXT2_SUPERBLOCK_START_LOCATION, ret, sizeof(ext2_superblock_t));
	if (((ext2_superblock_t*)ret)->major_version >= 1) {
		// @todo: expand the superblock and add the extensionfields
		kfree(ret);
		ret = (ext2_superblock_t*) kmalloc(sizeof(ext2_superblock_with_extended_t));
		read(EXT2_SUPERBLOCK_START_LOCATION, ret, sizeof(ext2_superblock_with_extended_t));
	} 
	return ret;
}




/**
 * @brief      Returns the filesystem block size
 *
 * @param      sb    superblock
 *
 * @return     Filesystem block size
 */
uint32_t ext2_get_block_size(ext2_superblock_t *sb) {
	return 1024 << sb->block_size_UNCALC;
}

/**
 * @brief      Function to initialize all the block group desciptors
 *
 * @param[in]  read  The read function ponter
 * @param      sb    pointer to superblock
 *
 * @return     pointer to linked list of block group descriptors
 */
ext2_block_group_descriptor_t *_ext2_get_all_block_group_descriptors(fs_read_fpointer read, ext2_superblock_t* sb){
	int number_of_blockgroups = roundup(sb->total_blocks, sb->blocks_in_blockgroup);
	ext2_block_group_descriptor_t *ret = (ext2_block_group_descriptor_t*) kmalloc(number_of_blockgroups*sizeof(ext2_block_group_descriptor_t));
	uint32_t offset = EXT2_SUPERBLOCK_START_LOCATION + ext2_get_block_size(sb);
	for (int i = 0; i < number_of_blockgroups; i++){
		read(offset, (void*) (ret + i*sizeof(ext2_block_group_descriptor_t)), sizeof(ext2_block_group_descriptor_t));
		offset += sb->blocks_in_blockgroup*ext2_get_block_size(sb);
	}
	return ret;
}

#if 0

/**
 * @brief      Gets the amount of blockgroup descriptors.
 *
 * @param      fs_info  The file system information
 *
 * @return     The amount of blockgroup descriptors.
 */
int _ext2_get_amount_of_blockgroup_descriptors(filesystem_t *fs_info)
{
	return roundup((int) fs_info->superblock, (int) fs_info->superblock->blocks_in_blockgroup);
}

#endif

/**
 * @brief      Gets the inode type
 *
 * @param      inode  The inode
 *
 * @return     the type of the inode
 */
uint16_t _ext2_get_inode_type(ext2_inode_t *inode) {
	return inode->type_permissions & 0xF000;
}



/**
 * @brief      gets the block group of an inode
 *
 * @param[in]  inode  The inode
 *
 * @return     the block group of the inode
 */
blkcnt_t _ext2_get_inode_block_group(ino_t inode, blkcnt_t blockspergroup)
{
	return (inode -1) / blockspergroup;
}

/**
 * @brief      get the inode size
 *
 * @param      fs_info  The file system information
 *
 * @return     the inode size
 */
size_t _ext2_get_inode_size(filesystem_t *fs_info)
{
	if (((ext2_superblock_t* )fs_info->superblock)->major_version >= 1){
		return ((ext2_superblock_with_extended_t*) fs_info->superblock)->inode_size;
	}
	return EXT2_DEFAULT_INODE_SIZE;
}


/**
 * @brief      Gets a block group descriptor by index
 *
 * @param[in]  blockgroup  The blockgroup index
 * @param      fs_info     The file system information
 *
 * @return     Pointer to block group descriptor
 */
ext2_block_group_descriptor_t *_ext2_get_block_group_descriptor(blkcnt_t blockgroup, filesystem_t *fs_info)
{
	return (ext2_block_group_descriptor_t*) (fs_info->blockgroup_list + sizeof(ext2_block_group_descriptor_t)*blockgroup);

}

/** 
 * @brief      gets the block offset of an inode
 *
 * @param[in]  inode  The inode
 *
 * @return     the block offset of the inode
 */
offset_t _ext2_get_inode_offset(ino_t inode, filesystem_t *fs_info)
{

	int blockgroup = (inode - 1) / ((ext2_superblock_t*) fs_info->superblock)->inodes_in_blockgroup;
	int index = (inode-1) % ((ext2_superblock_t*) fs_info->superblock)->inodes_in_blockgroup;
	blkcnt_t containingblock = (index * _ext2_get_inode_size(fs_info)) / ext2_get_block_size((ext2_superblock_t*)fs_info->superblock);
	ext2_block_group_descriptor_t *bgd = _ext2_get_block_group_descriptor(blockgroup, fs_info);
	offset_t offset = bgd->start_inode_table;
	offset += containingblock;
	offset *= ext2_get_block_size((ext2_superblock_t*) fs_info->superblock);
	offset += (index % (ext2_get_block_size((ext2_superblock_t*)fs_info->superblock) / _ext2_get_inode_size(fs_info)))*_ext2_get_inode_size(fs_info);
	return offset;
}


/**
 * @brief      Gets an inode from disk
 *
 * @param[in]  inode    The inode
 * @param      fs_info  The file system information
 *
 * @return     pointer to inode struct
 */
ext2_inode_t *_ext2_get_inode(ino_t inode, filesystem_t *fs_info)
{
	ext2_inode_t *buf = (ext2_inode_t*) kmalloc(_ext2_get_inode_size(fs_info));        
	fs_info->read(_ext2_get_inode_offset(inode, fs_info), buf, _ext2_get_inode_size(fs_info));
	return buf;
}


/**
 * @brief      Writes a inode to a specific inode location
 *
 * @param      inode_pointer  The inode pointer
 * @param[in]  inode_index    The inode index
 * @param      fs_info        The file system information
 *
 * @return     amount of bytes written
 */
ssize_t _ext2_write_inode(ext2_inode_t *inode_pointer, ino_t inode_index, filesystem_t *fs_info)
{
	return fs_info->write(_ext2_get_inode_offset(inode_index, fs_info), inode_pointer, _ext2_get_inode_size(fs_info));
}

#if 0

/**
 * @brief      Gets the next empty inode.
 *
 * @param      fs_info  The file system information
 *
 * @return     The next empty inode.
 */
int ext2_get_next_empty_inode(filesystem_t *fs_info)
{
	for (int i = 0; i < _ext2_get_amount_of_blockgroup_descriptors(fs_info); i++) {
		uint32_t *bgd = (uint32_t*) _ext2_get_block_group_descriptor(i, fs_info);
		for (int i = 0; i < (ext2_get_block_size(fs_info) / 4); i++) {
			if (bgd[i] != 0xFFFFFFFF) {
				// open hole

			}
		}
	}
}

#endif

/**
 * @brief      gets the inode data offset
 *
 * @param      inode              The inode
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param      fs_info            The file system information
 *
 * @return     { description_of_the_return_value }
 */
uint32_t _ext2_get_inode_data_offset(ext2_inode_t *inode, blkcnt_t blockpointerindex, filesystem_t *fs_info)
{
	//(inode-1) / ( (ext2_superblock_t*) fs_info->superblock)->inodes_in_blockgroup
	return inode->direct_block_pointer[blockpointerindex]*ext2_get_block_size((ext2_superblock_t*)fs_info->superblock);

}


/**
 * @brief      reads a file data block
 *
 * @param      fs_info            The file system information
 * @param[in]  inode              The inode
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param      buf                The buffer
 *
 * @return     returns the offset to the file block on disk
 */
offset_t _ext2_read_file_block(ino_t inode, blkcnt_t blockpointerindex, void *buf, filesystem_t *fs_info)
{
	// @todo: this should also work with single, double and triple indirect pointer blocks!!!
	offset_t offset = _ext2_get_inode_data_offset(_ext2_get_inode(inode, fs_info), blockpointerindex, fs_info);
	if (offset == 0) {
		return 0;
	}
	fs_info->read(offset, buf, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	return offset;
}


/**
 * @brief      reads from a file in the filesystem
 *
 * @param[in]  inode    The inode to read
 * @param      buf      The buffer to write to
 * @param[in]  count    The amount of bytes to read
 * @param      fs_info  The file system information
 *
 * @return     the actual amount of bytes read
 */
ssize_t ext2_read_from_file(ino_t inode, void *buf, size_t count, filesystem_t *fs_info)
{

	int ret    = 0;
	int blocks = roundup(count, ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
	int rest   = count % ext2_get_block_size((ext2_superblock_t*) fs_info->superblock);

	// all the blocks that should be coppied completely (blocks-1)
	int i = 0;
	for (i = 0; i < (blocks-1); i++){
		if (_ext2_read_file_block(inode, i,
		   (void*) (((uint32_t)buf)+i*ext2_get_block_size((ext2_superblock_t*)fs_info->superblock)), fs_info) == 0){
			return ret; // trying to read more than there are blocks
		}
		ret += ext2_get_block_size((ext2_superblock_t*) fs_info->superblock);
	}

	if (ret == 0) {
		return -1;
	}

	if (rest == 0){
		// needs to copy one last full block
		rest = ext2_get_block_size((ext2_superblock_t*)fs_info->superblock);
	}

	char *tmp_buf = (char*) kmalloc(ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));

	if (_ext2_read_file_block(inode, blocks-1, tmp_buf, fs_info) == 0){
		return ret;
	}
	memcpy(buf+i*ext2_get_block_size((ext2_superblock_t*)fs_info->superblock), tmp_buf, rest);
	ret += rest;

	kfree(tmp_buf);
	return ret;
}

/**
 * @brief      vfs layer for reading a file
 *
 * @param      vfs_node  The vfs node
 * @param[in]  inode     The inode to read
 * @param      buf       The buffer to write to
 * @param[in]  count     The amount of bytes to read
 *
 * @return     the actual amount of bytes to read
 */
ssize_t ext2_vfs_read_from_file(vfs_node_t *vfs_node, ino_t inode, void *buf, size_t count)
{
	return ext2_read_from_file(inode, buf, count, vfs_node->fs_info);
}

/**
 * @brief      Opens a ext2 directory stream
 *
 * @param[in]  inode    The inode
 * @param[in]  fs_info  The file system information
 *
 * @return     returns a directory stream object (DIR)
 */
DIR *ext2_open_dir_stream(ino_t inode, filesystem_t *fs_info)
{
	if (_ext2_get_inode_type(_ext2_get_inode(inode, fs_info)) != EXT2_INODE_TYPE_DIR){
		return 0;
	}

	DIR *ret = (DIR*) kmalloc(sizeof(DIR));
	memset(ret, 0, sizeof(DIR));
	ret->fs_info = fs_info;
	ret->inode = inode;
	ret->filebuffer = (void*) kmalloc(ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	_ext2_read_file_block(inode, ret->blockpointerindex, ret->filebuffer, fs_info);
	//print_hex_dump(ret->filebuffer, 0x100);
	return ret;
}

DIR *ext2_vfs_open_dir_stream(vfs_node_t *vfs_node)
{
	return ext2_open_dir_stream(vfs_node->offset, vfs_node->fs_info);
}


/**
 * @brief      Closes a directory stream
 *
 * @param      dirp  Pointer to the directory stream
 *
 * @return     kfree return
 */
int ext2_close_dir_stream(DIR* dirp)
{
	return kfree(dirp);
}


/**
 * @brief      Reads contents from a directory stream
 *
 * @param      dirp  The directory stream pointer
 *
 * @return     a dirent struct (see POSIX)
 */
struct dirent *ext2_read_dir(DIR *dirp)
{
	if (dirp->next_direntry_offset >= 0x400){
		// needs to copy next block into filebuffer if possible
		dirp->blockpointerindex++;
		int ret = _ext2_read_file_block(dirp->inode, dirp->blockpointerindex, dirp->filebuffer, dirp->fs_info);
		if (ret == 0){
			return 0;
		}
	}

	ext2_directory_entry_t *tmp = (ext2_directory_entry_t*) (((uint32_t)dirp->filebuffer)+dirp->next_direntry_offset);
	
	if (tmp->total_size_of_entry == 0 || tmp->inode == 0 || tmp->name_length == 0){
		return 0;
	}

	dirp->dirent.d_ino = tmp->inode;
	dirp->dirent.d_name = (char*) &tmp->name;
	dirp->next_direntry_offset += tmp->total_size_of_entry;

	return &dirp->dirent;
}

uint8_t _ext2_inode_type_to_vfs_type(uint16_t type)
{
	if (type == EXT2_INODE_TYPE_DIR){
		return VFS_DIRECTORY;
	} else if (type == EXT2_INODE_TYPE_REG){
		return VFS_FILE;
	} else if (type == EXT2_INODE_TYPE_CHR){
		return VFS_CHARDEVICE;
	} else if (type == EXT2_INODE_TYPE_BLK){
		return VFS_BLOCKDEVICE;
	} else if (type == EXT2_INODE_TYPE_FIFO){
		return VFS_PIPE;
	} else if (type == EXT2_INODE_TYPE_LINK){
		return VFS_SYMLINK;
	} else if (type == EXT2_INODE_TYPE_SOCK){
		return VFS_SOCKET;
	} else {
		return 0;
	}
}

/**
 * @brief      Creates a vfs entry
 *
 * @param[in]  inode_index  The inode index
 * @param[in]  id           The node identifier
 * @param      fs_info      The file system information
 *
 * @return     pointer to the vfs_node struct
 */
vfs_node_t *ext2_vfs_entry(uint32_t inode_index, uint32_t id, filesystem_t *fs_info)
{
	ext2_inode_t *inode = _ext2_get_inode(inode_index, fs_info);
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	node->type = _ext2_inode_type_to_vfs_type(_ext2_get_inode_type(inode));
	node->permissions = inode->type_permissions & 0xFFF;
	node->uid  = inode->uid;
	node->gid = inode->gid;
	node->id  = id;
	node->offset = inode_index;
	node->filelength = inode->low_size;
	node->fs_info = fs_info;
	node->open = 0;
	node->close = 0;
	node->read = &ext2_vfs_read_from_file;
	node->write = 0;
	node->opendir = &ext2_vfs_open_dir_stream;
	node->closedir = &ext2_close_dir_stream;
	node->readdir = &ext2_read_dir;
	return node;
}

/**
 * @brief      Initialises the filesystem
 *
 * @param      name   The name of the filesystem
 * @param[in]  read   The read function pointer for the filesystem
 * @param[in]  write  The write function pointer for the filesystem
 *
 * @return     A pointer to a filesystem info struct
 */
filesystem_t *ext2_initialize_filesystem(char* name, fs_read_fpointer read, fs_write_fpointer write)
{
	filesystem_t *ret = (filesystem_t*) kmalloc(sizeof(filesystem_t));
	ret->name  = name;
	ret->type  = FS_EXT2;
	ret->superblock = (offset_t*) _ext2_get_superblock(read);
	ret->blockgroup_list = (offset_t*) _ext2_get_all_block_group_descriptors(read, (ext2_superblock_t*) ret->superblock);
	ret->start = EXT2_ROOT_INODE;
	ret->read  = read;
	ret->write = write;
	ret->file_read   = &ext2_read_from_file;
	ret->file_write  = 0;
	ret->dir_open    = &ext2_open_dir_stream;
	ret->dir_close   = &ext2_close_dir_stream;
	ret->dir_read 	 = &ext2_read_dir;
	ret->fs_makenode = &ext2_vfs_entry;
	return ret;
}


#if 0		// unnecessary testing function
void testing(){
	#if 0
	ext2_superblock_t *superblock = _getSuperblock();
	ext2_superblock_extended_t* superblockExtended = _getSuperBlockExtended();
	ext2_block_group_descriptor_t *bgd = (ext2_block_group_descriptor_t*) ramdisk_get(EXT2_SUPERBLOCK_START_LOCATION+1024);
	print("filesystem major version: "); print_int(superblock->major_version);
	print("filesystem minor version: "); print_int(superblock->minor_version);
	print("total inodes: "); print_int(superblock->total_inodes);print("\n");
	print("total blocks: "); print_int(superblock->total_blocks);print("\n");
	print("inode size:"); print_int(superblockExtended->inode_size);print("\n");
	print("block bitmap: "); print_hex(bgd->block_bitmap);print("\n");
	print("inode bitmap: "); print_hex(bgd->inode_bitmap);print("\n");
	print("start inode table: "); print_hex(bgd->start_inode_table); print("\n");
	//print_hex_dump((uint32_t*) ramdisk_get(0x400*bgd->start_inode_table), 0x200);
	//print_binary_char(*((uint8_t*) ramdisk_get(0x400*5)+9));
	print("\nblock size: "); print_int(1024 << superblock->block_size_UNCALC);
	ext2_inode_t *root = (ext2_inode_t*) ramdisk_get(bgd->start_inode_table*0x400+superblockExtended->inode_size);
	print("\nroot low size: "); print_int(root->low_size);
	print("\nroot type: "); print_hex(root->type_permissions & 0xF000);
	print("\nroot block pointer 1: "); print_hex(root->direct_block_pointer[0]);print("\n");
	//print_hex_dump((uint32_t*) ramdisk_get(0x400*root->direct_block_pointer[0]), 0x200);
	print("---------------------------------------------------------------\n");

	//_loop_over_directory(root);
	
	print("---------------------------------------------------------------\n");


	
	uint32_t addr = kmalloc(1024);
	
	memset((uint32_t*) addr, 0, 1024);
	
	ramdisk_read(root->direct_block_pointer[0]*0x400, (uint32_t*) addr, 1024);



	DIR *dirp = ext2_open_dir_stream(2, fs_info);
	if (dirp == 0){
		print("error");
		return;
	}
	struct dirent *dir = 0;
	while ((dir = ext2_read_dir(dirp)) != 0){
		print("the direntry inode: "); print_int(dir->d_ino); print(" the direntry name: '");print(dir->d_name);print("'\n");
	}
	print("ret");
	//#endif
	#endif

	filesystem_t *fs_info = initialize_filesystem("EXT2FS", ramdisk_read, ramdisk_write);

	print("reading from file (inode: 12) \n");

	char *buf = (char*) kmalloc(0x401);
	if (ext2_read_from_file(12, buf, 0x401, fs_info) == -1){
		print("error");
	}
	print("buffer content: \n");print(buf);
	
}

#endif


/*

functions that should be created:
	- loop over directory
	- read from file descriptor
	- write to file descriptor
	- open file descriptor
	- close file descriptor

*/


