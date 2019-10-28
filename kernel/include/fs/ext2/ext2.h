#ifndef EXT2_DRIVER_
#define EXT2_DRIVER_

#include <fs/fs.h>

#include <stdint.h>
#include <stddef.h>

/* Structure definitions: */ 

/**
 * EXT2 super block structure as defined in
 * https://www.nongnu.org/ext2-doc/ext2.html#superblock
 * and
 * https://wiki.osdev.org/Ext2
 */
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

	// following are extended fields which we won't use yet

	uint32_t		first_non_reserved_inode;
	uint16_t		inode_size;
	uint16_t		block_group_of_this_superblock;
	
	uint32_t		optional_feature_flag;
	uint32_t		required_feature_flag;
	uint32_t		must_required_feature_flag;	// if not present must be mounted read only
	
	uint8_t			fs_id[16];					// output of blkid
	uint8_t			volume_name[16];			// c string
	uint8_t			last_mount_path[64];		// c string
	
	uint32_t		compression_algorithms;
	
	uint8_t			blocks_to_preallocate_files;
	uint8_t			blocks_to_preallocate_dirs;

	uint16_t		UNUSED;

	uint8_t			journal_id[16];				// same as fs_id 
	uint32_t		journal_inode;
	uint32_t		journal_device;
	uint32_t		orphan_inode_list_head;

} __attribute__((packed)) ext2_superblock_t;

/**
 * The EXT2 block group descriptor structure
 */
typedef struct {
	uint32_t 		block_bitmap; 			// this is the block address
	uint32_t 		inode_bitmap;			// this is the block address
	uint32_t 		start_inode_table; 		// this is the block address
	uint16_t 		unallocated_block_count;// in group
	uint16_t 		unallocated_inode_count;// in group
	uint16_t 		directory_count;
	uint8_t			unused[14];
} __attribute__((packed)) ext2_block_group_descriptor_t;

/**
 * The EXT2 inode structure
 */
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

}__attribute__((packed)) ext2_inode_t;

/**
 * The EXT2 Directory entry structure
 */
typedef struct {
	uint32_t 		inode;
	uint16_t		total_size_of_entry;	// total size of this directory entry
	uint8_t			name_length;			// max amount of chars will be 256 because of size of name length field (1 byte)
	uint8_t 		type_indicator;
	char 			name;			// start of name
	// ... name characters
} __attribute__((packed)) ext2_directory_entry_t;

/* Function declarations: */

/**
 * @brief      Initializes the ext2 file system
 *
 * @param      name   The name
 * @param[in]  read   The read
 * @param[in]  write  The write
 *
 * @return     Pointer to the file system info structure
 */
filesystem_t *init_ext2_filesystem(char *name, fs_read_fpointer read, fs_write_fpointer write);


#endif
