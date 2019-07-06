/**
 * fs/ext2/ext2.c
 * 
 * Author: Robbe De Greef
 * Date:   21 may 2019
 * 
 * Version 1.0
 */

#include <drivers/ramdisk/ramdisk.h>
#include <drivers/video/videoText.h>
#include <lib/string/string.h>
#include <lib/math/math.h>
#include <drivers/vfs/vfs.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <fs/fs.h>
#include <mm/heap.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>


// @todo file open function should be created to initialize vfs_nodes

/**
 * @brief      Gets a pointer to the superblock.
 *
 * @param[in]  read  The read function pointer
 *
 * @return     Pointer to the superblock.
 */
ext2_superblock_t *_ext2_get_superblock(fs_read_fpointer read){

	// we just read the maximum amount of fields because it doesn't matter if they are there or not 
	// we will just use them if they are and if we use a little more memory well so be it then
	ext2_superblock_t *ret = (ext2_superblock_t*) kmalloc(sizeof(ext2_superblock_with_extended_t));
	read(EXT2_SUPERBLOCK_START_LOCATION, ret, sizeof(ext2_superblock_with_extended_t));
	

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
	//print("fs info loc: "); print_hex((uint32_t) fs_info);
	//print("\nblocksize: "); print_hex(ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
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
 * @brief      Gets the offset of a block
 *
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param      fs_info            The file system information
 *
 * @return     The offset
 */
uint32_t _ext2_get_block_offset(blkcnt_t blockpointerindex, filesystem_t *fs_info)
{
	return blockpointerindex * ext2_get_block_size((ext2_superblock_t*)fs_info->superblock);
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
	ext2_inode_t *in = _ext2_get_inode(inode, fs_info);
	offset_t offset = _ext2_get_inode_data_offset(in, blockpointerindex, fs_info);
	if (offset == 0) {
		return 0;
	}
	fs_info->read(offset, buf, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	return offset;
}

/**
 * @brief      Writes to a file data block
 *
 * @param      fs_info            The file system information
 * @param[in]  inode              The inode
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param      buf                The buffer
 *
 * @return     returns the offset to the file block on disk
 */
offset_t _ext2_write_file_block(ino_t inode, blkcnt_t blockpointerindex, const void *buf, filesystem_t *fs_info)
{
	// @todo: this should also work with single, double and triple indirect pointer blocks!!!
	offset_t offset = _ext2_get_inode_data_offset(_ext2_get_inode(inode, fs_info), blockpointerindex, fs_info);
	if (offset == 0) {
		return 0;
	}
	fs_info->write(offset, buf, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	return offset;
}

/**
 * @brief      Reads from a block into memory
 *
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param      buf                The buffer
 * @param      fs_info            The file system information
 *
 * @return     Returnvalue of read
 */
ssize_t _ext2_read_block(blkcnt_t blockpointerindex, void *buf, filesystem_t *fs_info)
{
	offset_t offset = _ext2_get_block_offset(blockpointerindex, fs_info);
	if (offset == 0) {
		return 0;
	}
	return fs_info->read(offset, buf, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));	
}

/**
 * @brief      Writes a block to the filesystem
 *
 * @param[in]  blockpointerindex  The blockpointerindex
 * @param[in]  buf                The buffer
 * @param      fs_info            The file system information
 *
 * @return     Returnvalue of write
 */
ssize_t _ext2_write_block(blkcnt_t blockpointerindex, const void *buf, filesystem_t *fs_info)
{
	offset_t offset = _ext2_get_block_offset(blockpointerindex, fs_info);
	if (offset == 0) {
		return 0;
	}
	return fs_info->write(offset, buf, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
}


#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

ino_t _ext2_alloc_inode(filesystem_t *fs_info)
{
	// find first empty inode
	for (size_t bgs = 0; bgs < (unsigned int) roundup(((ext2_superblock_t*) fs_info->superblock)->total_blocks, ((ext2_superblock_t*) fs_info->superblock)->blocks_in_blockgroup); bgs++) {
		ext2_block_group_descriptor_t *bgd = _ext2_get_block_group_descriptor(bgs, fs_info);
		
		if (bgd->unallocated_inode_count != 0) {
			uint32_t *bitmap = (uint32_t*) kmalloc(ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
			fs_info->read(bgd->inode_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock), bitmap, 
		  	ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));

			for (size_t i = 0; i < ext2_get_block_size((ext2_superblock_t*)fs_info->superblock) / 32; i++) {
				if (bitmap[i] != 0xFFFFFFFF) {
					for (size_t j = 0; j < 32; j++) {
						unsigned int bit = 1 << j;
						if ((bitmap[i] & bit) == 0) {
							// free frame
							ino_t inode = i*32 +j;
							// set used
							bitmap[i] |= bit;
							bgd->unallocated_inode_count--;
							// write back to block group descriptor
							fs_info->write(bgd->inode_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock),
										   (const void*) bitmap, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
							kfree(bitmap);
							
							return inode;
						}
					}

				}
			}
		}
		
	}
	errno = ENOSPC;
	// no inode found
	return 0;
}

void _ext2_dealloc_inode(uint32_t inode, filesystem_t *fs_info)
{
	ext2_block_group_descriptor_t *bgd = _ext2_get_block_group_descriptor(
										 _ext2_get_inode_block_group(inode, ((ext2_superblock_t*) fs_info->superblock)->inodes_in_blockgroup),
										  fs_info);
	uint32_t *bitmap = (uint32_t*) kmalloc(ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
	fs_info->read(bgd->inode_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock), bitmap, 
				  ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	// this sets the node to unused (xor the byte)
	bitmap[inode / 32] ^= (1 << (inode % 32));
	fs_info->write(bgd->inode_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock),
									   (const void*) bitmap, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));

}

blkcnt_t _ext2_alloc_block(filesystem_t *fs_info)
{
	// find first empty inode
	for (size_t bgs = 0; bgs < (unsigned int) roundup(((ext2_superblock_t*) fs_info->superblock)->total_blocks, ((ext2_superblock_t*) fs_info->superblock)->blocks_in_blockgroup); bgs++) {
		ext2_block_group_descriptor_t *bgd = _ext2_get_block_group_descriptor(bgs, fs_info);
		
		if (bgd->unallocated_block_count != 0) {
			uint32_t *bitmap = (uint32_t*) kmalloc(ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
			fs_info->read(bgd->block_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock), bitmap, 
		  				  ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));

			for (size_t i = 0; i < ext2_get_block_size((ext2_superblock_t*)fs_info->superblock) / 32; i++) {
				if (bitmap[i] != 0xFFFFFFFF) {
					for (size_t j = 0; j < 32; j++) {
						unsigned int bit = 1 << j;
						if ((bitmap[i] & bit) == 0) {
							// free frame
							ino_t inode = i*32 +j;
							// set used
							bitmap[i] |= bit;
							bgd->unallocated_block_count--;
							// write back to block group descriptor
							fs_info->write(bgd->inode_bitmap * ext2_get_block_size((ext2_superblock_t* ) fs_info->superblock),
										   (const void*) bitmap, ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
							kfree(bitmap);
							
							return inode;
						}
					}

				}
			}
		}
		
	}
	errno = ENOSPC;
	// no inode found
	return 0;
}

blkcnt_t _ext2_add_block_to_inode(ino_t inode_index, blkcnt_t blockindex, filesystem_t *fs_info)
{
	// @todo: this should also work with double and tripple indirect block pointers!!!
	ext2_inode_t *inode = _ext2_get_inode(inode_index, fs_info);

	for (size_t i = 0; i < 12; i++) {
		if (inode->direct_block_pointer[i] == 0) {
			inode->direct_block_pointer[i] = blockindex;
			_ext2_write_inode(inode, inode_index, fs_info);

			kfree(inode);
			return i;
		}
	}

	kfree(inode);
	return 0;
}

int ext2_add_dir(uint32_t parent_inode_index, uint32_t inode, char *name, uint16_t type_indicator, filesystem_t *fs_info)
{
	

	// now write inode into parent dir

	// get the directory
	DIR *dirp = ext2_open_dir_stream(parent_inode_index, fs_info);	
	struct dirent *dir;
	if (dirp != 0) {

		// go to last entry
		while ((dir = ext2_read_dir(dirp)) != 0) {
	
		}
			
		// now create the direntry
		// 	- calulate the size of the entry
		size_t sz = sizeof(ext2_directory_entry_t) + sizeof(char) * (strlen(name)+1);
		//  - now create the node 
		ext2_directory_entry_t *direntry = (ext2_directory_entry_t*) kmalloc(sz);
		memset(direntry, 0, sz);

		//  - set it's contents
		direntry->inode = inode;
		direntry->total_size_of_entry = sz;
		direntry->name_length = strlen(name);
		direntry->type_indicator = type_indicator;
		memcpy(&direntry->name, name, strlen(name)+1);
	
		// get last entry
		//  - if the filebuffer is already full allocate new one
	
	
		ext2_directory_entry_t *lastdirent = (ext2_directory_entry_t*) (dirp->next_direntry_offset - dirp->lastentrysize + (uint32_t) dirp->filebuffer);
		
		int lastdirentsz = sizeof(ext2_directory_entry_t) + sizeof(char) * (strlen(&lastdirent->name)+1);
		if (sz + lastdirentsz <= dirp->lastentrysize) {
			// it fits
			direntry->total_size_of_entry = dirp->lastentrysize - sz;
			lastdirent->total_size_of_entry = lastdirentsz;
	
			memcpy((void*) ((uint32_t) lastdirent + lastdirentsz), direntry, sz);
	
	
			_ext2_write_file_block(parent_inode_index, dirp->blockpointerindex-1, (const char*) dirp->filebuffer, fs_info);
			kfree(direntry);

			return 0;
		} else {
			// create new block
			// (later in this code)
		}
		
	}
	print("creating new block");
	// it doesn't, we need to create new block
	// @todo: create new block
	blkcnt_t newblockindex = _ext2_alloc_block(fs_info);
	_ext2_add_block_to_inode(parent_inode_index, newblockindex, fs_info);
	if (newblockindex == 0) {
		return -1;
	}

	size_t sz = sizeof(ext2_directory_entry_t) + sizeof(char) * (strlen(name)+1);
	//  - now create the node 
	ext2_directory_entry_t *direntry = (ext2_directory_entry_t*) kmalloc(sz);
	memset(direntry, 0, sz);

	//  - set it's contents
	direntry->inode = inode;
	direntry->total_size_of_entry = ext2_get_block_size((ext2_superblock_t*) fs_info->superblock) - sz;
	direntry->name_length = strlen(name);
	direntry->type_indicator = type_indicator;
	memcpy(&direntry->name, name, strlen(name)+1);

	void *buf = kmalloc(ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
	memset(buf, 0, ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
	memcpy(buf, direntry, sz);

	_ext2_write_block(newblockindex, (const char*) buf, fs_info);
	kfree(buf);
	kfree(direntry);
	return 0;
}

uint16_t _ext2_vfs_to_inode_type(uint8_t type)
{
	if (type == VFS_DIRECTORY) {
		return EXT2_INODE_TYPE_DIR;
	} else if (type == VFS_CHARDEVICE) {
		return EXT2_INODE_TYPE_CHR;
	} else if (type == VFS_BLOCKDEVICE) {
		return EXT2_INODE_TYPE_BLK;
	} else if (type == VFS_PIPE) {
		return EXT2_INODE_TYPE_FIFO;
	} else if (type == VFS_SOCKET) {
		return EXT2_INODE_TYPE_SOCK;
	} else if (type == VFS_SYMLINK) {
		return EXT2_INODE_TYPE_LINK;
	} else {
		return EXT2_INODE_TYPE_REG;
	} 
}

ino_t _ext2_create_inode(ino_t parent_inode_index, char *name, uint16_t type, uint16_t permissions, uint16_t uid, 
								 uint16_t gid, uint16_t size, uint16_t flags, void *buf, filesystem_t *fs_info)
{
	// first try to add the inode into the parent inode
	// if this fails it means we shouldn't continue with creating the inode
	ino_t newinode_index = _ext2_alloc_inode(fs_info);

	if (ext2_add_dir(parent_inode_index, newinode_index, name, type, fs_info) == -1) {
		// deallocate the newinode
		_ext2_dealloc_inode(newinode_index, fs_info);
		return 0;
	}

	ext2_inode_t *newinode = (ext2_inode_t *) kmalloc(sizeof(ext2_inode_t));
	memset(newinode, 0, sizeof(ext2_inode_t));
	newinode->type_permissions = type | permissions;
	newinode->uid = uid;
	newinode->gid = gid;
	newinode->size = size;
	newinode->flags = flags;
	if (size == 0) {
		newinode->disk_sectors = 0;
	} else {
		if ((size % ext2_get_block_size((ext2_superblock_t*) fs_info->superblock)) == 0) {
			newinode->disk_sectors = (size / ext2_get_block_size((ext2_superblock_t*) fs_info->superblock));
		} else {
			newinode->disk_sectors = (size / ext2_get_block_size((ext2_superblock_t*) fs_info->superblock)) + 1;
		}
	}

	(void) (buf);
	//@todo: set disk sectors and write into memory

	_ext2_write_inode(newinode, newinode_index, fs_info);

	return newinode_index;
}

/**
 * @brief      Creates a inode
 *
 * @param      node   The node
 * @param      name   The name
 * @param[in]  flags  The flags
 *
 * @return     Offset (inode index)
 */
offset_t ext2_create_node_vfs(vfs_node_t *node, char *name, uint16_t flags)
{
	return _ext2_create_inode(node->parent->offset, name, _ext2_vfs_to_inode_type(node->type), node->permissions,
							  node->uid, node->gid, 0, flags, 0, node->fs_info);
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

	//if (ret == 0) {
	//	return -1;
	//}

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
	ret->dirent.d_name = (char*) kmalloc(NAME_MAX);	// name buffer
	ret->fs_info = fs_info;
	ret->inode = inode;
	ret->filebuffer = (void*) kmalloc(ext2_get_block_size((ext2_superblock_t*)fs_info->superblock));
	//print("filebuffer: " ); print_hex((uint32_t) ret->filebuffer);
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
		dirp->next_direntry_offset = 0;
	}

	ext2_directory_entry_t *tmp = (ext2_directory_entry_t*) (((uint32_t)dirp->filebuffer)+dirp->next_direntry_offset);
	if (tmp->inode == 0) {
		// this should NEVER happen
		return 0;
	}

	dirp->dirent.d_ino = tmp->inode;
	memcpy(dirp->dirent.d_name, &tmp->name, tmp->name_length);
	dirp->dirent.d_name[tmp->name_length] = '\0'; 				// string ends with 0 char
	dirp->next_direntry_offset += tmp->total_size_of_entry;
	dirp->lastentrysize = tmp->total_size_of_entry;
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
	if (inode_index == 0) {
		for(;;);
	} 
	ext2_inode_t *inode = _ext2_get_inode(inode_index, fs_info);
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));
	node->type = _ext2_inode_type_to_vfs_type(_ext2_get_inode_type(inode));
	node->permissions = inode->type_permissions & 0xFFF;
	node->uid  = inode->uid;
	node->gid = inode->gid;
	node->id  = id;
	node->offset = inode_index;
	node->filelength = inode->size ;
	node->fs_info = fs_info;
	node->open = 0;
	node->close = 0;
	node->read = &ext2_vfs_read_from_file;
	node->write = 0;
	node->creat = &ext2_create_node_vfs;
	node->opendir = &ext2_vfs_open_dir_stream;
	node->closedir = &ext2_close_dir_stream;
	node->readdir = &ext2_read_dir;
	kfree(inode);
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


