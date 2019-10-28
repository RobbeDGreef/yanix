#ifndef _DIRENT_H
#define _DIRENT_H

#include <fs/fs.h>
#include <sys/types.h>

struct filesystem_s;
typedef struct filesystem_s filesystem_t;


/**
 * @brief      dirent structure according to POSIX standards (+ d_length but this does not interfere with POSIX)
 */
struct dirent {
	ino_t	 	d_ino;		// the inode number
	char 		*d_name;	// file name
	size_t 		d_length;	// name length
};


typedef struct DIR_s {
	filesystem_t	*fs_info;
	offset_t		next_direntry_offset; 	// offset in file of next direntry
	ino_t	 		inode;
	blkcnt_t 		blockpointerindex;
	size_t 			lastentrysize;
	void			*filebuffer;
	struct dirent  	dirent;
} DIR;

#endif