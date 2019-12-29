/**
 * @defgroup   FS ext2
 *
 * @brief      This file implements a very minimal ext2 driver.
 *
 * @author     Robbe De Greef
 * @date       2019
 */

#include <types.h>
#include <mem/mem.h>
#include <load.h>

/* The KERNEL_INODE and KERNEL_ENTRY macro are defined when compiling */
#define SUPERBLOCK_POS 	0x0400
#define EXT2_SIGNATURE 	0xEF53
#define ROOT_INODE 		0x0002
#define INODE_SIZE 		0x0080
#define PLACEMENT_START 0xF000

unsigned int global_offset = 0;


/* Superblock structure */
#pragma pack(1)
typedef struct {
	uint32_t 		total_inodes;
	uint32_t 		total_blocks;
	uint32_t 		total_reserved_for_superuser;
	uint32_t 		total_unallocated_blocks;
	uint32_t 		total_unallocated_inodes;
	uint32_t 		block_number_of_superblock;
	
	uint32_t 		block_size_frag;		// to calc real block size shift 1024 this amount of times to the left
	uint32_t 		fragment_size_frag;		// idem

	uint32_t 		blocks_in_blockgroup;
	uint32_t 		fragments_in_blockgroup;
	uint32_t 		inodes_in_blockgroup;
	uint32_t 		last_mount_time;		// POSIX time
	uint32_t 		last_written_time;		// POSIX time

	uint16_t		mount_amount_since_last_check;	// consistency check (fsck)
	uint16_t		max_mounts_after_check;

	uint16_t		ext2_signature;	// (0xef53) just to help confirm presence of ext2 on a volume
	uint16_t		fs_state;		// 1 clean  / 2 has errors
	uint16_t		when_error; 	// 1 ignore / 2 remount fs as read-only / 3 kernel panic
	uint16_t		minor_version;

	uint32_t		last_check_time;			// POSIX time
	uint32_t		forced_check_interval;		// POSIX time
	uint32_t		os_creator_id;				// 0 linux / 1 GNU HURD / 2 MASIX / 3 FREEBSD / 4 other bsd derivatives
	uint32_t		major_version;

	uint16_t		uid;
	uint16_t		gid;

	/* This implementation only supports > 1.0 ext2 implementations so extended superblock is not necessary */

} superblock_t;

/**
 * The EXT2 inode structure
 */
#pragma pack(1)
typedef struct {
	uint16_t		type_permissions;	// type and permissions
	uint16_t 		uid;
	uint32_t 		size;				// lower 32 bytes of size
	uint32_t 		last_access_time; 	// posix time
	uint32_t 		creation_time; 		// posix time
	uint32_t 		last_mod_time; 		// posix time
	uint32_t 		deletion_time; 		// posix time

	uint16_t		gid;
	
	uint16_t		hard_link_count;
	uint32_t 		disk_sectors; 		// does not count inode struct nor directory entries linkint to the inode

	uint32_t 		flags;				
	uint32_t 		os_specific_value_1;

	uint32_t 		direct_block_pointer[12];
	uint32_t 		single_indirect_block_pointer;
	uint32_t 		double_indirect_block_pointer;
	uint32_t 		triple_indirect_block_pointer;

	uint32_t 		gen_number;
	uint32_t 		reserved;
	uint32_t 		reserved2;
	uint32_t 		block_addr_fragment;
	uint32_t 		os_specific_value_2[3]; // 3*4 bytes -> 12 bytes

} inode_t;

/**
 * The EXT2 block group descriptor structure
 */
#pragma pack(1)
typedef struct {
	uint32_t 		block_bitmap; 			// this is the block address
	uint32_t 		inode_bitmap;			// this is the block address
	uint32_t 		start_inode_table; 		// this is the block address
	uint16_t 		unallocated_block_count;// in group
	uint16_t 		unallocated_inode_count;// in group
	uint16_t 		directory_count;
	uint8_t			unused[14];
} bgd_t;

unsigned int _inode_offset_block(unsigned int inode_index, superblock_t *sb)
{
	return ((inode_index - 1) % sb->inodes_in_blockgroup * INODE_SIZE / (1024 << sb->block_size_frag));
}

unsigned int _inode_offset_rel_to_block(unsigned int inode_index, superblock_t *sb)
{
	return ((inode_index - 1) % sb->inodes_in_blockgroup % ((1024 << sb->block_size_frag) / INODE_SIZE)) * INODE_SIZE; 
}

inode_t *get_inode_ref(unsigned int inode_index, unsigned int startinode, unsigned int blocksize, superblock_t *sb, unsigned int disk)
{
	void *buf = malloc(1024 << sb->block_size_frag, 0);
	
	print("location: "); print_int(global_offset + _inode_offset_block(inode_index, sb) * ((1024 << sb->block_size_frag) / blocksize)); print("\n");
	
	load_block(disk, global_offset + (_inode_offset_block(inode_index, sb) + startinode) * ((1024 << sb->block_size_frag) / blocksize), (1024 << sb->block_size_frag) / blocksize, (unsigned int) buf, 0);
	
	return (inode_t*) (((unsigned int) buf) + _inode_offset_rel_to_block(inode_index, sb));
}

void load_ext2_block(unsigned int block, unsigned int placement_address, superblock_t *sb, unsigned int bs, unsigned int disk)
{
	load_block(disk, global_offset + block * ((1024 << sb->block_size_frag) / bs), (1024 << sb->block_size_frag) / bs, placement_address % 0x10, placement_address / 0x10);
}

/**
 * @brief      Handles indirect block pointer indexes
 *
 * @param[in]  indirect_bp_index  The indirect bp index
 * @param[in]  block              The block
 * @param      sb                 The superblock
 * @param[in]  bs                 The physical block size
 * @param[in]  disk               The disk
 *
 * @return     The block location of the requested block
 */
unsigned int _handle_indirirect_bp(unsigned int indirect_bp_index, unsigned int block, superblock_t *sb, unsigned int bs, unsigned int disk)
{
	/* Create a buffer */
	uint32_t *indirect_bp = (uint32_t*) malloc(1024 << sb->block_size_frag, 1);

	print(" "); print_int(indirect_bp);

	/* Load the block into the buffer */
	load_ext2_block(indirect_bp_index, (uint32_t) indirect_bp, sb, bs, disk);

	/* Extract block value */
	unsigned int ret = indirect_bp[block];

	/* Cleanup */
	free(0x1000);

	return ret;
}

unsigned int _ext2_get_block_loc(inode_t *inode, unsigned int block, superblock_t *sb, unsigned int blocksize, unsigned int disk)
{
	if (block < 12)
	{
		return inode->direct_block_pointer[block];
	}
	else
	{
		/* 4 is the size of an uint32_t because the blockpointers are saved in a uint32_t array */
		if (block < (1024 << sb->block_size_frag) / 4 + 12)
		{
			return _handle_indirirect_bp(inode->single_indirect_block_pointer, block - 12, sb, blocksize, disk);
		}
		else if (block < (unsigned int) pow((1024 << sb->block_size_frag) / 4, 2) + 12)
		{
			/* Double indirect bp */
			print("Double indirect\n");
			return 0;
		}
		else
		{
			return 0;
		}
	}
}

void _load_kernel(inode_t *kernel_inode, superblock_t *sb, unsigned int blocksize, unsigned int disk)
{
	unsigned int total_block_count = kernel_inode->size / (1024 << sb->block_size_frag);
	unsigned int placement_address = PLACEMENT_START;	/* Just a guess */

	print("block count: "); print_int(total_block_count);

	unsigned int copy_iter = 0;

	print("Kernel size: "); print_int(kernel_inode->size); print("\n");

	/* Load in the kernel */
	/* This system loads in every half MB of the kernel image into memory and copies it into the correct location, and repeats */
	for (size_t i = 0; i < total_block_count; i++)
	{
		unsigned int x = _ext2_get_block_loc(kernel_inode, i, sb, blocksize, disk);
		print(" "); print_int(x);
		if (x == 0)
		{
			print("Too large ?");
			break;
		}

		load_ext2_block(x, placement_address, sb, blocksize, disk);
		placement_address += (1024 << sb->block_size_frag);
		
		/* If we copied half a meg */
		if (placement_address == (placement_address + 512 * 1024))
		{
			memcpy(KERNEL_ENTRY + (512 * 1024 * copy_iter), PLACEMENT_START, 512*1024);
			copy_iter++;
			placement_address = PLACEMENT_START;
		}
	}

	memcpy(KERNEL_ENTRY + (512 * 1024 * copy_iter), PLACEMENT_START, placement_address - PLACEMENT_START);
	print("placement_address: "); print_int(placement_address); print("\n");
}

/**
 * @brief      Loads the kernel file from a ext2 filesystem.
 *
 * @param      kernel_name  The kernel name
 * @param[in]  startlba     The startlba
 * @param[in]  total_lba    The total lba
 * @param[in]  disk         The disk
 *
 * @return     Whether the kernel loaded successfully or not
 */
int load_ext2_kernel(char *kernel_name, unsigned int startlba, unsigned int total_lba, unsigned int disk, unsigned int blocksize_phys)
{
	print("Loading kernel from ext2 filesystem\n");

	/* We don't have to shift or mask any bytes because the superblock is aligned at 1024 we just have to make sure we load in the correct lba */
	unsigned int superblock_lba = startlba + (SUPERBLOCK_POS / blocksize_phys);

	global_offset = startlba;
	
	/* Lets create a buffer */
	superblock_t *sb = (superblock_t*) malloc(sizeof(superblock_t), 1);

	bgd_t *bgd = malloc(sizeof(bgd_t), 0);

	/* load the superblock */
	load_block(disk, superblock_lba, 1, (unsigned int) sb, 0);
	load_block(disk, superblock_lba + (1024 << sb->block_size_frag) / blocksize_phys, 1, (unsigned int) bgd, 0);

	print("Superblock loaded successfully\n");

	if (sb->ext2_signature != EXT2_SIGNATURE)
	{
		print("Cannot load kernel, not a ext2 filesystem or superblock was loaded incorrectly\n");
		return 1;
	}

	/* Get the inode reference to the kernel inode */
	inode_t *inode_refrence = get_inode_ref(KERNEL_INODE, bgd->start_inode_table, blocksize_phys, sb, disk);
	
	print("Starting to load kernel\n");

	_load_kernel(inode_refrence, sb, blocksize_phys, disk);

}