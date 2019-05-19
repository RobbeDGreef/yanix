#ifndef EXT2_DRIVER
#define EXT2_DRIVER

#include <fs/dirent.h>
#include <stdint.h>


#define EXT2_SUPERBLOCK_START_LOCATION 	1024
#define EXT2_SUPERBLOCK_LENGTH 			1024


#define EXT2_FS_CLEAN 	1
#define EXT2_FS_ERRORS	2

#define EXT2_IGNORE_ERR		1
#define EXT2_REMOUNT_R		2
#define EXT2_KERNEL_PANIC	3

#define EXT2_OS_ID_LINUX	0
#define EXT2_OS_ID_GNU_HURD	1
#define EXT2_OS_ID_MASIX	2
#define EXT2_OS_ID_FREEBSD	3
#define EXT2_OS_ID_OTHERBSD	4

typedef struct {
	uint32_t 		total_inodes;
	uint32_t 		total_blocks;
	uint32_t 		total_reserved_for_superuser;
	uint32_t 		total_unallocated_blocks;
	uint32_t 		total_unallocated_inodes;
	uint32_t 		block_number_of_superblock;
	
	uint32_t 		block_size_UNCALC;		// to calc real block size shift 1024 times to left
	uint32_t 		fragment_size_UNCALC;	// idem

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

} __attribute__((packed)) ext2_superblock_t;

#define EXT2_DEFAULT_NON_RESERVED_INODE	11
#define EXT2_DEFAULT_INODE_SIZE			128

typedef struct {
	uint32_t 		total_inodes;
	uint32_t 		total_blocks;
	uint32_t 		total_reserved_for_superuser;
	uint32_t 		total_unallocated_blocks;
	uint32_t 		total_unallocated_inodes;
	uint32_t 		block_number_of_superblock;
	
	uint32_t 		block_size_UNCALC;		// to calc real block size shift 1024 times to left
	uint32_t 		fragment_size_UNCALC;	// idem

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

	// following are extended fields

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

} __attribute__((packed)) ext2_superblock_with_extended_t;

typedef struct {
	uint32_t 		block_bitmap; 			// this is the block address
	uint32_t 		inode_bitmap;			// this is the block address
	uint32_t 		start_inode_table; 		// this is the block address
	uint16_t 		unallocated_block_count;// in group
	uint16_t 		unallocated_inode_count;// in group
	uint16_t 		directory_count;
	uint8_t			unused[14];
	// unused for 14 bytes
} __attribute__((packed)) ext2_block_group_descriptor_t;


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

typedef struct {
	uint16_t		type_permissions;	// type and permissions
	uint16_t 		uid;
	uint32_t 		low_size;			// lower 32 bytes of size
	uint32_t 		last_access_time; 	// possix time
	uint32_t 		creation_time; 		// possix time
	uint32_t 		last_mod_time; 		// possix time
	uint32_t 		deletion_time; 		// possix time

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


#define EXT2_DIR_ENTRY_TYPE_UNKNOWN 			0x00
#define EXT2_DIR_ENTRY_TYPE_REGULAR 			0x01
#define EXT2_DIR_ENTRY_TYPE_DIRECTORY			0x02
#define EXT2_DIR_ENTRY_TYPE_CHARACTER_DEVICE 	0x03
#define EXT2_DIR_ENTRY_TYPE_BLOCK_DEVICE	 	0x04
#define EXT2_DIR_ENTRY_TYPE_FIFO 				0x05
#define EXT2_DIR_ENTRY_TYPE_SOCKET 				0x06
#define EXT2_DIR_ENTRY_TYPE_SYMBOLIC_LINK 		0x07


typedef struct {
	uint32_t 		inode;
	uint16_t		total_size_of_entry; 			// total size of this directory entry
	uint8_t			name_length;					// max amount of chars will be 256 because of size of name length field (1 byte)
	uint8_t 		type_indicator;
	char* 			name;
	// ... name characters
} __attribute__((packed)) ext2_directory_entry_t;



void testing();

/**
 * @brief      Reads contents from a directory stream
 *
 * @param      dirp  The directory stream pointer
 *
 * @return     a dirent struct (see POSIX)
 */
struct dirent *ext2_read_dir(DIR *dirp);

/**
 * @brief      Opens a ext2 directory stream
 *
 * @param[in]  fs_info  The file system information
 * @param[in]  inode    The inode
 *
 * @return     returns a directory stream object (DIR)
 */
DIR *ext2_open_dir_stream(ino_t inode, filesystem_t *fs_info);


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
ssize_t ext2_read_from_file(ino_t inode, void *buf, size_t count, filesystem_t *fs_info);

/**
 * @brief      Initialises the filesystem
 *
 * @param      name   The name of the filesystem
 * @param[in]  read   The read function pointer for the filesystem
 * @param[in]  write  The write function pointer for the filesystem
 *
 * @return     A pointer to a filesystem info struct
 */
filesystem_t *ext2_initialize_filesystem(char* name, fs_read_fpointer read, fs_write_fpointer write);

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
ssize_t ext2_vfs_read_from_file(vfs_node_t *vfs_node, ino_t inode, void *buf, size_t count);

/**
 * @brief      Creates a vfs entry
 *
 * @param[in]  inode_index  The inode index
 * @param[in]  id           The node identifier
 * @param      fs_info      The file system information
 *
 * @return     pointer to the vfs_node struct
 */
vfs_node_t *ext2_vfs_entry(ino_t inode_index, id_t id, filesystem_t *fs_info);

/**
 * @brief      Closes a directory stream
 *
 * @param      dirp  Pointer to the directory stream
 *
 * @return     kfree return
 */
int ext2_close_dir_stream(DIR* dirp);

#endif