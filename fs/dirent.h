#ifndef _DIRENT_H
#define _DIRENT_H

#include <fs/fs.h>
#include <sys/types.h>

struct filesystem_s;
typedef struct filesystem_s filesystem_t;


#define NAME_MAX 	255


/**
 * @brief      dirent structure according to POSIX standards
 */
struct dirent {
	ino_t	 	d_ino;		// the inode number
	char 		*d_name;	// file name
};


typedef struct DIR_s {
	filesystem_t	*fs_info;
	offset_t		next_direntry_offset; 	// offset in file of next direntry
	ino_t	 		inode;
	blkcnt_t 		blockpointerindex;
	void			*filebuffer;
	struct dirent  	dirent;
} DIR;

#endif