#include <fs/fs.h>
#include <fs/ext2/ext2.h>
#include <fs/dirent.h>
#include <mm/heap.h>

#include <libk/string.h>
#include <libk/math.h>
#include <libk/bit.h>

#include <stdint.h>
#include <errno.h>
#include <const.h>
#include <drivers/disk.h>

#include <debug.h>

/**
 * Some definitions to avoid magic numbers
 */
#define EXT2_SUPERBLOCK_START_LOC 	1024
#define EXT2_SUPERBLOCK_LENGTH 		1024

#define EXT2_FS_CLEAN 1
#define EXT2_FS_DIRTY 0

#define EXT2_IGNORE_ERR		1
#define EXT2_REMOUNT_R		2
#define EXT2_KERNEL_PANIC	3

#define EXT2_OS_ID_LINUX	0
#define EXT2_OS_ID_GNU_HURD	1
#define EXT2_OS_ID_MASIX	2
#define EXT2_OS_ID_FREEBSD	3
#define EXT2_OS_ID_OTHERBSD	4

#define EXT2_DEFAULT_NON_RESERVED_INODE	11
#define EXT2_DEFAULT_INODE_SIZE			128

/**
 * Inode definitions
 */
#define EXT2_INODE_FLAG_SECURE_DELETION	 				0x01
#define EXT2_INODE_FLAG_KEEP_COPY_AFTER_DELETE			0x02
#define EXT2_INODE_FLAG_FILE_COMPRESSION 				0x04
#define EXT2_INODE_FLAG_SYNC_UPDATES 					0x08 	// new data is written immediately to disk
#define EXT2_INODE_FLAG_IMMUTABLE_FILE 					0x10 	// content cannot be changed
#define EXT2_INODE_FLAG_APPEND_ONLY 					0x20 
#define EXT2_INODE_FLAG_NOT_IN_DUMP 					0x40
#define EXT2_INODE_FLAG_LAST_ACCESSED_TIME_NO_UPDATE	0x80 	// last access time should never be updated
#define EXT2_INODE_FLAG_HASH_INDEXED_DIR 				0x10000
#define EXT2_INODE_FLAG_AFS_DIR 						0x20000
#define EXT2_INODE_FLAG_JOURNAL_FILE_DATA 				0x40000

#define EXT2_INODE_TYPE_SOCK 	0xc000		// socket
#define EXT2_INODE_TYPE_LINK 	0xa000		// symbolic link
#define EXT2_INODE_TYPE_REG 	0x8000		// regular file
#define EXT2_INODE_TYPE_BLK 	0x6000		// block device
#define EXT2_INODE_TYPE_DIR 	0x4000 		// directory
#define EXT2_INODE_TYPE_CHR 	0x2000 		// character device
#define EXT2_INODE_TYPE_FIFO 	0x1000 		// fifo

#define EXT2_ROOT_INODE 		0x02
#define EXT2_SIGNATURE 			0xef53

#define EXT2_DIRECT_BLOCK_POINTER_AMOUNT 	12

/**
 * Directory entry definitions
 */
#define EXT2_DIR_ENTRY_TYPE_UNKNOWN 			0x00
#define EXT2_DIR_ENTRY_TYPE_REGULAR 			0x01
#define EXT2_DIR_ENTRY_TYPE_DIRECTORY			0x02
#define EXT2_DIR_ENTRY_TYPE_CHARACTER_DEVICE 	0x03
#define EXT2_DIR_ENTRY_TYPE_BLOCK_DEVICE	 	0x04
#define EXT2_DIR_ENTRY_TYPE_FIFO 				0x05
#define EXT2_DIR_ENTRY_TYPE_SOCKET 				0x06
#define EXT2_DIR_ENTRY_TYPE_SYMBOLIC_LINK 		0x07

/**
 * @brief      Calculates the block size and returns it
 *
 * @param      superblock  The superblock
 *
 * @return     The block size
 */
static inline unsigned int ext2_block_size(ext2_superblock_t *superblock)
{
	/* Calculate the block size and return it */
	return 1024 << superblock->block_size_frag;
}

ssize_t ext2_disk_read(unsigned long offset, void *buf, size_t size, filesystem_t *fs_info)
{
	return disk_read(offset + fs_info->partstart, buf, size, fs_info->disk_info);
}

/**
 * @brief      Copies the ext2 superblock into memory
 *
 * @param[in]  read  The read function pointer
 *
 * @return     Memory location of the superblock
 */
void *_ext2_copy_superblock_into_memory(unsigned int start, disk_t *disk_info)
{
	ext2_superblock_t *ret = (ext2_superblock_t*) kmalloc(sizeof(ext2_superblock_t));
	disk_read(EXT2_SUPERBLOCK_START_LOC + start, ret, sizeof(ext2_superblock_t), disk_info);
	return (void*) ret;
}

/**
 * @brief      Copies all the block group descriptors into memory
 *
 * @param[in]  read        The read
 * @param      superblock  The superblock
 *
 * @return     { description_of_the_return_value }
 */
ext2_block_group_descriptor_t *_ext2_get_all_block_group_descriptors(ext2_superblock_t *superblock, unsigned int fs_start, disk_t *disk_info)
{
	/* First calculate the amount of block groups */
	unsigned int number_of_blockgroups = roundup(superblock->total_blocks, superblock->blocks_in_blockgroup);

	/* Now allocate the appropriate space in memory for all of our blockgroups */
	ext2_block_group_descriptor_t *bgd_array = kmalloc(sizeof(ext2_block_group_descriptor_t) * number_of_blockgroups);

	// offset of first block group descriptor
	unsigned int bgd_offset = EXT2_SUPERBLOCK_START_LOC + ext2_block_size(superblock);
	disk_read(bgd_offset + fs_start, bgd_array, sizeof(ext2_block_group_descriptor_t) * number_of_blockgroups, disk_info);
	
	return bgd_array;

}

/**
 * @brief      Returns the inode size
 *
 * @param      fs_info  The file system information
 *
 * @return     Inode size
 */
unsigned int _ext2_get_inode_size(filesystem_t *fs_info)
{
	if (((ext2_superblock_t *) fs_info->superblock)->major_version >= 1)
	{
		/* This ext2 version supports superblock extensions and thus holds the inode size*/
		return ((ext2_superblock_t *) fs_info->superblock)->inode_size;
	} 
	else
	{
		/* This ext2 doesn't hold the superblock extensions and thus the inode size will be default */
		return EXT2_DEFAULT_INODE_SIZE;
	}
}

unsigned int _ext2_get_inode_offset(ino_t inode, filesystem_t *fs_info)
{
	/* This is only defined to reduce code clutter in this function you could calculate this inline */
	ext2_superblock_t *superblock = (ext2_superblock_t*) fs_info->superblock;

	/* First we've got to figure out what blockgroup this inode belongs to */
	unsigned int blockgroup = (inode - 1) / superblock->inodes_in_blockgroup;

	/* After that calculate the index of the inode in this bg */
	unsigned int index = (inode - 1) % superblock->inodes_in_blockgroup;

	/* And now the block that contains the inode */
	unsigned int containing_block = (index * _ext2_get_inode_size(fs_info) / fs_info->block_size);

	/* Get a reference to the containing block */
	ext2_block_group_descriptor_t *bgd_ref = &((ext2_block_group_descriptor_t*) fs_info->blockgroup_list)[blockgroup];

	/* Now we will calculate the actual location of the inode */
	unsigned int inode_offset = bgd_ref->start_inode_table;

	/* Add the containing block index to the offset */
	inode_offset += containing_block;
	inode_offset *= fs_info->block_size;
	inode_offset += (index % (fs_info->block_size / _ext2_get_inode_size(fs_info))) * _ext2_get_inode_size(fs_info);

	return inode_offset;
}

/**
 * @brief      Returns a inode structure
 *
 * @param[in]  inode    The inode
 * @param      fs_info  The file system information
 *
 * @return     The inode reference structure
 */
ext2_inode_t *_ext2_get_inode_ref(ino_t inode, filesystem_t *fs_info)
{
	/* First calculate the inode size */
	unsigned int inode_size = _ext2_get_inode_size(fs_info);

	/* Now allocate the appropriate buffer size */
	ext2_inode_t *inode_ref = (ext2_inode_t*) kmalloc(inode_size);

	/* Now copy the inode data into our inode structure */
	ext2_disk_read(_ext2_get_inode_offset(inode, fs_info), inode_ref, inode_size, fs_info);
	return inode_ref;
}

static unsigned int handle_indirect_bp(unsigned int indirect_bp_index, unsigned int block_to_read, filesystem_t *fs_info)
{
	/* Create a buffer for the block pointer block and copy data */
	uint32_t *indirect_bp = (uint32_t*) kmalloc(fs_info->block_size);

	if (indirect_bp == 0)
	{
		return 0;
	}

	ext2_disk_read(indirect_bp_index * fs_info->block_size, indirect_bp, fs_info->block_size, fs_info);

	unsigned int ret = indirect_bp[block_to_read] * fs_info->block_size;

	/**
	 * @todo:	Figure out why this is happening and also if this is common, figure out if linux does this 
	 * 			type of fixing too.
	 *
	 * @bug: 	Okay so ext2 is doing something weird and i can't find any information on the issue online
	 * 			So for some reason it sometimes just randomly skips a block in it's indirect block pointer
	 * 			structure. Why? I have no clue. It looks like this
	 * 			
	 * 			C0220080:  AD 0C 00 00 AE 0C 00 00   AF 0C 00 00 B0 0C 00 00
	 *			C0220090:  B1 0C 00 00 B2 0C 00 00   B3 0C 00 00 00 00 00 00    <--- here
	 *			C02200A0:  B5 0C 00 00 B6 0C 00 00   B7 0C 00 00 B8 0C 00 00
	 *			
	 *			As you can see (these are all 4 byte integer values) the blockpointer index for block
	 *			0xcb4 (probably) is missing, i've looked at the raw hex code of the disk and there 
	 *			actually is data in that block, and from the looks of it that data is just a continuation 
	 *			of a normal elf file so it all looks like just this one block pointer is going bananas.
	 *			
	 *			I've checked my drivers and even the raw disk.iso file has this issue so it isn't any weird
	 *			os / memory bug or something, this actually is something else.
	 *			
	 *			The thing I don't understand is it is just all of a sudden popping up and other strange things are 
	 *			happening too like for example my executables also being able to run on my linux machine. So it
	 *			might just be some weiiird ass toolchain stuff that's happening. I might need to rebuild everything 
	 *			i don't know but for now i'll try to fix it with this workaround  (see next XXX)
	 *	
	 *	
	 *			
	 * @XXX:	So as we just discussed something in ext2 is doing weird stuff so I created this weird workaround
	 * 			that checks if the next blockpointer index is valid and if the value that should be guessed is guessable
	 * 			
	 * @XXX:	Okay so 2 days later and i get another one only this time it's two blockpointers that are missing
	 * 			what the hell is going on???? I'll fix this one up as well but I've got a feeling three blockpointer gaps are comming
	 */

	if (!ret)
	{
		if (indirect_bp[block_to_read+1] || indirect_bp[block_to_read+2])
		{
			if ((indirect_bp[block_to_read+1] == indirect_bp[block_to_read-1]+2) || (indirect_bp[block_to_read+2] == indirect_bp[block_to_read-1]+3))
			{
				int fill = indirect_bp[block_to_read-1] + 1;

				printk(KERN_WARNING "Filled in blockindex gap with: %i\n", fill);
				ret = fill * fs_info->block_size;
			}
			else if (indirect_bp[block_to_read+1] == indirect_bp[block_to_read-2]+3)
			{
				int fill = indirect_bp[block_to_read-2] + 2;

				printk(KERN_WARNING "Filled in blockindex gap with: %i\n", fill);
				ret = fill * fs_info->block_size;
			}
		}
	}

	/* cleanup */
	kfree(indirect_bp);

	return ret;
}

unsigned int _ext2_get_inode_data_offset(ext2_inode_t *inode, unsigned int  block_to_read, filesystem_t *fs_info)
{
	/**
	 * If we need more than 12 (EXT2_DIRECT_BLOCK_POINTER_AMOUNT)
	 * we will have to make use of the indirect block pointers
	 */
	if (block_to_read < EXT2_DIRECT_BLOCK_POINTER_AMOUNT)
	{
		return inode->direct_block_pointer[block_to_read] * fs_info->block_size;
	} 
	else
	{
		if (block_to_read < fs_info->block_size / sizeof(uint32_t) + EXT2_DIRECT_BLOCK_POINTER_AMOUNT)
		{
			/* Single indirect block pointer will be used */
			return handle_indirect_bp(inode->single_indirect_block_pointer, block_to_read - EXT2_DIRECT_BLOCK_POINTER_AMOUNT, fs_info);
		} 
		else if (block_to_read  < (unsigned int) pow(fs_info->block_size / sizeof(uint32_t), 2) + EXT2_DIRECT_BLOCK_POINTER_AMOUNT)
		{
			/* Double indirect block pointer will be used */

			/* First the double indirect block pointer */
			
			/* Create a buffer and copy data */
			uint32_t *d_indirect_bp = (uint32_t*) kmalloc(fs_info->block_size);
			
			if (d_indirect_bp == 0)
			{
				return 0;
			}
			
			if (inode->double_indirect_block_pointer == 0)
			{
				return 0;
			}

			ext2_disk_read(inode->double_indirect_block_pointer * fs_info->block_size, d_indirect_bp, fs_info->block_size, fs_info);


			/* calculate the location of our single indirect bp */
			unsigned int indirect_bp_index = (block_to_read - (fs_info->block_size / sizeof(uint32_t)) - EXT2_DIRECT_BLOCK_POINTER_AMOUNT) / (fs_info->block_size / sizeof(uint32_t));
			unsigned int block_index 	   = (block_to_read - (fs_info->block_size / sizeof(uint32_t)) - EXT2_DIRECT_BLOCK_POINTER_AMOUNT) % (fs_info->block_size / sizeof(uint32_t));


			/* Check which block we need to read */
			unsigned int indirect_to_read = d_indirect_bp[indirect_bp_index];
			/* Is it present */
			if (indirect_to_read == 0)
			{
				/* Cleanup */
				kfree(d_indirect_bp);
				return 0;
			}
			
			unsigned int ret = handle_indirect_bp(indirect_to_read, block_index, fs_info);


			/* Cleanup */
			kfree(d_indirect_bp);

			return ret;
		} 
		else if (block_to_read <= (unsigned int) pow(fs_info->block_size / sizeof(uint32_t), 3))
		{
			/* Triple indirect block pointer will be used */
			printk("error, ext2, reading data, file too large/not implemented yet\n");
			return 0;
		} else{
			printk("error, ext2, reading data, file too large?\n");
			/* File to large? */
			return 0;
		}
	}
}

/**
 * @brief      Copies tha data from a block index from a given inode into a given buffer
 *
 * @param      inode          The inode
 * @param[in]  block_to_read  The block to read
 * @param      buf            The buffer
 * @param      fs_info        The file system information
 *
 * @return     Amount of bytes actually written
 */
unsigned int _ext2_read_inode_block(ext2_inode_t *inode, unsigned int block_to_read, void *buf, filesystem_t *fs_info)
{

	unsigned int data_offset = _ext2_get_inode_data_offset(inode, block_to_read, fs_info);
	if (data_offset == 0)
	{
		/* I don't know any better errno value for this */
		errno = EFBIG;
		return 0;
	}

	return ext2_disk_read(data_offset, buf, fs_info->block_size, fs_info);
}

ssize_t _ext2_write_block(unsigned int block, void *buf, filesystem_t *fs_info)
{
	return disk_write(block * fs_info->block_size, buf, fs_info->block_size, fs_info->disk_info);
}

/**
 * @brief      Writes an inode into the filesystem
 *
 * @param      inode_ref  The inode reference
 * @param[in]  inode      The inode
 * @param      fs_info    The file system information
 *
 * @return     Return from the write
 */
ssize_t _ext2_write_inode(ext2_inode_t *inode_ref, ino_t inode, filesystem_t *fs_info)
{
	return disk_write(_ext2_get_inode_offset(inode, fs_info), inode_ref, _ext2_get_inode_size(fs_info), fs_info->disk_info);
}

/**
 * @brief      Reads data from a file into a given buffer
 *
 * @param[in]  inode       The inode
 * @param      buf         The buffer
 * @param[in]  byte_count  The byte count
 * @param      fs_info     The file system information
 *
 * @return     The actual amount of bytes written.
 */
ssize_t ext2_read_from_file(ino_t inode, unsigned int offset, void *buf, size_t byte_count, filesystem_t *fs_info)
{
	/* The return value, this will be the amount of bytes copied */
	unsigned int ret = 0;

	/* Calculate the amount of blocks and extra bytes needed to copy */
	unsigned int block_count  = byte_count / fs_info->block_size;
	unsigned int byte_rest 	  = byte_count % fs_info->block_size;

	/* Get a reference to the wanted inode */
	ext2_inode_t *inode_ref = _ext2_get_inode_ref(inode, fs_info);

	/* Error check */
	if (inode_ref == 0)
	{
		return -1;
	}

	unsigned int startblock = offset / fs_info->block_size;
	unsigned int start_rest = offset % fs_info->block_size;

	if (start_rest)
	{
		void *tmp_buf = kmalloc(fs_info->block_size);

		if (tmp_buf == 0)
			return -1;

		if (_ext2_read_inode_block(inode_ref, startblock, tmp_buf, fs_info) == 0)
		{
			printk(KERN_DEBUG "Error reading inode block\n");
			return -1;
		}

		memcpy(buf, (void*) ((unsigned int) tmp_buf + offset), fs_info->block_size - start_rest);

		kfree(tmp_buf);

		ret += fs_info->block_size - start_rest;

		startblock++;
	}

	/**
	 * loop over those blocks and copy them 
	 */
	for (size_t i = startblock; i < block_count+startblock; i ++)
	{
		if (_ext2_read_inode_block(inode_ref, i, (void*) ((unsigned int) buf + ret) , fs_info) == 0)
		{
			/* Error occurred, we tried to read more blocks then there are */
			printk("Error, tried to read block: %u addr: %08x errno: %i\n", i, (char*)buf + i * fs_info->block_size, errno);

			/* Cleanup */
			kfree(inode_ref);

			return ret;
		}
		/* Copied a block successfully so update our return value */
		ret += fs_info->block_size;
	}

	/** 
	 * If we have a byte rest we need to copy one more block 
	 * but we have to make sure we don't overwrite the buffer
	 */
	if (byte_rest)
	{
		/* Create a temporally buffer */
		void *tmp_buf = kmalloc(fs_info->block_size);
			
		if (tmp_buf == 0)
		{
			return -1;
		}

		/* Copy the data */
		if (_ext2_read_inode_block(inode_ref, block_count, tmp_buf, fs_info) == 0)
		{
			printk(KERN_DEBUG "error reading inode block errno %i\n", errno);
			return -1;
		}

		
		/* Now copy the correct amount into the real buffer */
		memcpy((void*) ((unsigned int) buf + ret), tmp_buf, byte_rest);

		/* Cleanup */
		kfree(tmp_buf);

		/* Update written byte var */
		ret += byte_rest;
	}

	/* Cleanup */
	kfree(inode_ref);

	return ret;

}

ssize_t ext2_vfs_read_from_file(vfs_node_t *vfs_node, unsigned int offset, void *buf, size_t count)
{
	return ext2_read_from_file(vfs_node->offset, offset, buf, count, vfs_node->fs_info);
}

/**
 * @brief      Converts ext2 type values to the vfs type values
 *
 * @param[in]  type  The type
 *
 * @return     The converted type
 */
unsigned char _ext2_inode_type_to_vfs_type(unsigned short type)
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
 * @brief      The inverse of the function above
 *
 * @param[in]  type  The type
 *
 * @return     The converted type
 */
unsigned short _ext2_vfs_type_to_inode_type(unsigned char type)
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


/**
 * @brief      Reads a directory stream
 *
 * @param      dirp  The dirp
 *
 * @return     A dirent struct with the appropriate values
 */
struct dirent *ext2_read_dir(DIR *dirp)
{
	/* We need to copy a new block into memory of our direntry offset exceeds bounds (block size) */
	if (dirp->next_direntry_offset >= dirp->fs_info->block_size)
	{
		dirp->blockpointerindex++;

		ext2_inode_t *inode_ref = _ext2_get_inode_ref(dirp->inode, dirp->fs_info);

		/* Try to copy next block */
		if (_ext2_read_inode_block(inode_ref, dirp->blockpointerindex, dirp->filebuffer, dirp->fs_info) == 0)
		{
			/* Failed to copy next block */
			errno = 0;
			return 0;
		}
		dirp->next_direntry_offset = 0;
	}



	/* Grab our next direntry reference */
	ext2_directory_entry_t *tmp = (ext2_directory_entry_t*) (((uint32_t) dirp->filebuffer) + dirp->next_direntry_offset);

	if (tmp->inode == 0)
	{
		//printk("Should never happen really");
		/* This should normally never happen... I think? It isn't really clear to me */
		return 0;
	} 

	/* Copy name */
	memcpy(&dirp->dirent.d_name, &tmp->name, tmp->name_length);
	dirp->dirent.d_name[tmp->name_length] = '\0'; 				// string ends with 0 char
	
	/* @todo: we don't really need dirp anymore with the new dirent offset fields so i might need to redo this system (or complete ext2 because it's a mess) */

	dirp->next_direntry_offset += tmp->total_size_of_entry;
	dirp->lastentrysize = tmp->total_size_of_entry;
	
	/* Fill in the dirp structure with correct data and make ready for next cycle */
	dirp->dirent.d_ino = tmp->inode;
	dirp->dirent.d_off = dirp->next_direntry_offset;
	dirp->dirent.d_reclen = tmp->name_length + sizeof(struct dirent)+1;
	dirp->dirent.d_type = _ext2_inode_type_to_vfs_type(tmp->type_indicator);

	return &dirp->dirent;
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
	/* Check whether inode is a directory or not */
	ext2_inode_t *inode_ref = _ext2_get_inode_ref(inode, fs_info);
	if ((inode_ref->type_permissions & EXT2_INODE_TYPE_DIR) == 0)
	{
		/* The given inode is not a directory */
		return 0;
	}

	/* Allocate the needed structures and fill them in */
	DIR *ret = (DIR*) kmalloc(sizeof(DIR) + NAME_MAX);
	memset(ret, 0, sizeof(DIR));

	//ret->dirent.d_name = (char*) kmalloc(NAME_MAX);
	//ret->dirent.d_name = "";
	ret->fs_info = fs_info;
	ret->inode = inode;
	ret->filebuffer = (void*) kmalloc(fs_info->block_size);

	/* Now read in the first block of this directory */
	_ext2_read_inode_block(inode_ref, 0, ret->filebuffer, fs_info);


	/* Cleanup */
	kfree(inode_ref);

	return ret;
}

/**
 * @brief      Opens vfs directory stream (stub for vfs) 
 *
 * @param      vfs_node  The vfs node
 *
 * @return     A pointer to a directory stream
 */
DIR *ext2_vfs_open_dir_stream(vfs_node_t *vfs_node)
{
	return ext2_open_dir_stream(vfs_node->offset, vfs_node->fs_info);
}

/**
 * @brief      Closes a directory stream 
 *
 * @param      dirp  The dirp
 *
 * @return     free() return
 */
int ext2_close_dir_stream(DIR* dirp)
{
	return kfree(dirp);
}

/**
 * @brief      Allocates a block in the given block
 *
 * @param[in]  inode    The inode
 * @param      fs_info  The file system information
 *
 * @return     { description_of_the_return_value }
 */
unsigned int _ext2_alloc_block(filesystem_t *fs_info)
{
	/* Calculate the amount of blockgroups */
	ext2_superblock_t *superblock = (ext2_superblock_t*) fs_info->superblock;
	unsigned int bgd_amount = roundup(superblock->total_blocks, superblock->blocks_in_blockgroup);

	/* Create buffer for info */
	uint32_t *block_bitmap = (uint32_t*) kmalloc(fs_info->block_size);
	unsigned int free_block = 0;

	/* Loop over every blockgroup and look for a empty block somewhere */
	for (size_t bgd_i = 0; bgd_i < bgd_amount; bgd_i++)
	{
		
		/* Copy data into buffer */
		unsigned int block_bitmap_location = (((ext2_block_group_descriptor_t*) fs_info->blockgroup_list)[bgd_i].block_bitmap) * fs_info->block_size; 
		disk_read(block_bitmap_location, block_bitmap, fs_info->block_size, fs_info->disk_info);
	
		/* Loop to find empty block */
		for (size_t i = 0; i < fs_info->block_size / 4; i++)
		{
			if (block_bitmap[i] != 0xFFFFFFFF)
			{
				/* Empty block spotted */
				for (size_t j = 0; j < 32; j++)
				{
					if (getbit(block_bitmap[i], j) == 0)
					{
						/* Empty block found */
						block_bitmap[i] = block_bitmap[i] | (1 << j);
						free_block = i * 32 + j;
				
						/* Write the found block back to the block bitmap */
						disk_write(block_bitmap_location, block_bitmap, fs_info->block_size, fs_info->disk_info);

						goto block_found;
					}
				}
			}
		}
	}

block_found:

	/* Cleanup */
	kfree(block_bitmap);

	return free_block;
}

ino_t _ext2_alloc_inode(filesystem_t *fs_info)
{
	/* Calculate the amount of blockgroups */
	ext2_superblock_t *superblock = (ext2_superblock_t*) fs_info->superblock;
	unsigned int bgd_amount = roundup(superblock->total_blocks, superblock->blocks_in_blockgroup);

	/* Create buffer for info */
	uint32_t *inode_bitmap = (uint32_t*) kmalloc(fs_info->block_size);
	unsigned int free_inode = 0;

	/* Loop over every blockgroup and look for a empty block somewhere */
	for (size_t bgd_i = 0; bgd_i < bgd_amount; bgd_i++)
	{
		
		/* Copy data into buffer */
		unsigned int block_bitmap_location = (((ext2_block_group_descriptor_t*) fs_info->blockgroup_list)[bgd_i].inode_bitmap) * fs_info->block_size; 
		ext2_disk_read(block_bitmap_location, inode_bitmap, fs_info->block_size, fs_info);
	
		/* Loop to find empty block */
		for (size_t i = 0; i < fs_info->block_size / 4; i++)
		{
			if (inode_bitmap[i] != 0xFFFFFFFF)
			{
				/* Empty block spotted */
				for (size_t j = 0; j < 32; j++)
				{
					if (getbit(inode_bitmap[i], j) == 0)
					{
						/* Empty block found */
						inode_bitmap[i] = inode_bitmap[i] | (1 << j);
						free_inode = i * 32 + j;
				
						/* Write the found block back to the block bitmap */
						disk_write(block_bitmap_location, inode_bitmap, fs_info->block_size, fs_info->disk_info);

						goto block_found;
					}
				}
			}
		}
	}

block_found:

	/* Cleanup */
	kfree(inode_bitmap);

	return free_inode;
}

void _ext2_dealloc_inode(uint32_t inode, filesystem_t *fs_info)
{
	(void) (inode);
	(void) (fs_info);
}

int ext2_add_dir(uint32_t parent_inode, uint32_t inode, char *name, uint16_t type_indicator, filesystem_t *fs_info)
{
	(void) (parent_inode);
	(void) (inode);
	(void) (name);
	(void) (type_indicator);
	(void) (fs_info);
	return 0;
}



ino_t _ext2_create_inode(ino_t parent_inode, char *name, uint16_t type, uint16_t permissions, uint16_t uid, 
								 uint16_t gid, uint16_t size, uint16_t flags, void *buf, filesystem_t *fs_info)
{
	/* First allocate a new inode structure */
	ino_t new_inode = _ext2_alloc_inode(fs_info);

	/* Try to add our new inode in the directory system */
	if (ext2_add_dir(parent_inode, new_inode, name, type, fs_info) == -1)
	{
		/* Failed to add our inode into the directory system so just exit here */
		_ext2_dealloc_inode(new_inode, fs_info);
		return 0;
	}

	/* Create the necessary buffer */
	ext2_inode_t *new_inode_ref = (ext2_inode_t*) kmalloc(sizeof(ext2_inode_t));
	memset(new_inode_ref, 0, sizeof(ext2_inode_t));

	/* Fill in our buffer / inode_structure */
	new_inode_ref->type_permissions = type | permissions;
	new_inode_ref->uid 				= uid;
	new_inode_ref->gid 				= gid;
	new_inode_ref->size 			= size;
	new_inode_ref->flags 			= flags;

	/* Check whether the inode needs data sectors or not */
	if (size == 0)
	{
		new_inode_ref->disk_sectors = 0;
	}
	else
	{
		if ((size % fs_info->block_size) == 0)
		{
			new_inode_ref->disk_sectors = (size / fs_info->block_size);
		}
		else
		{
			new_inode_ref->disk_sectors = (size / fs_info->block_size) + 1;
		}
	}

	/* Now allocate and set disk sectors and write the inode into memory */
	for (size_t i = 0; i < new_inode_ref->disk_sectors; i++)
	{
		unsigned int block = _ext2_alloc_block(fs_info);
		if (block == 0)
		{
			break;
		}
		_ext2_write_block(block, buf + i * fs_info->block_size, fs_info);
	}

	_ext2_write_inode(new_inode_ref, new_inode, fs_info);

	return new_inode;
}


/**
 * @brief      Creates a inode in the system
 *
 * @param      node   The node
 * @param      name   The name
 * @param[in]  flags  The flags
 *
 * @return     Offset (inode index)
 */
offset_t ext2_create_node_vfs(vfs_node_t *node, char *name, uint16_t flags)
{
	return _ext2_create_inode(node->parent->offset, name, _ext2_vfs_type_to_inode_type((unsigned char) node->type), node->permissions,
							  node->uid, node->gid, 0, flags, 0, node->fs_info);
}

/**
 * @brief      Creates a vfs entry
 *
 * @param[in]  inode    The inode index
 * @param      name     The name
 * @param[in]  id       The node identifier
 * @param      fs_info  The file system information
 *
 * @return     pointer to the vfs_node structure
 */
vfs_node_t *ext2_vfs_entry(uint32_t inode, char *name, uint32_t id, filesystem_t *fs_info)
{
	/* If we got a illegal inode value return */
	if (inode == 0)
	{
		return 0;
	} 


	/* Copy our inode structure into memory and create a vfs node structure */
	ext2_inode_t *inode_ref = _ext2_get_inode_ref(inode, fs_info);
	
	if (inode_ref == 0)
		return 0;
	
	vfs_node_t *node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));
	memset(node, 0, sizeof(vfs_node_t));

	/* Fill in our vfs node structure with the appropriate values */
	node->name 			= name;
	node->type 			= _ext2_inode_type_to_vfs_type(inode_ref->type_permissions & 0xF000);
	node->permissions 	= inode_ref->type_permissions & 0xFFF;
	node->uid  			= inode_ref->uid;
	node->gid 			= inode_ref->gid;
	node->id  			= id;
	node->offset 		= inode;
	node->filelength 	= inode_ref->size ;
	node->fs_info 		= fs_info;
	node->open 			= 0;
	node->close 		= 0;
	node->read 			= &ext2_vfs_read_from_file;
	node->write 		= 0;
	node->creat 		= &ext2_create_node_vfs;
	node->opendir 		= &ext2_vfs_open_dir_stream;
	node->closedir 		= &ext2_close_dir_stream;
	node->readdir 		= &ext2_read_dir;

	/* Cleanup */	
	kfree(inode_ref);

	return node;
}


/**
 * @brief      Initializes the ext2 file system
 *
 * @param      name   The name
 * @param[in]  read   The read
 * @param[in]  write  The write
 *
 * @return     Pointer to the file system info structure
 */
filesystem_t *init_ext2_filesystem(char *name, unsigned int ext2_start, disk_t *disk_info)
{
	filesystem_t *ret = (filesystem_t *) kmalloc(sizeof(filesystem_t));
	ret->name = name;
	ret->type = FS_EXT2;
	ret->superblock = (void*) _ext2_copy_superblock_into_memory(ext2_start, disk_info);
	
	if (((ext2_superblock_t*) ret->superblock)->ext2_signature != EXT2_SIGNATURE)
	{
		return 0;
	}

	ret->blockgroup_list = (void*) _ext2_get_all_block_group_descriptors((ext2_superblock_t*) ret->superblock, ext2_start, disk_info);
	
	ret->block_size = ext2_block_size((ext2_superblock_t*) ret->superblock);

	ret->disk_info = disk_info;
	ret->start = EXT2_ROOT_INODE;
	ret->partstart = ext2_start;
	ret->file_read = &ext2_read_from_file;
	ret->file_write = 0;
	ret->dir_open = &ext2_open_dir_stream;
	ret->dir_close = &ext2_close_dir_stream;
	ret->dir_read = &ext2_read_dir;
	ret->fs_makenode = &ext2_vfs_entry;

	return ret;
}