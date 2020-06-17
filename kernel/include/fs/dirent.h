#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <sys/types.h>

struct filesystem_s;
typedef struct filesystem_s filesystem_t;

/**
 * @brief      dirent structure according to POSIX standards
 */
struct dirent
{
	unsigned long  d_ino;    /* The inode number */
	unsigned long  d_off;    /* The offset to the next dirent */
	unsigned short d_reclen; /* Length of this dirent structure in bytes */

	// char		pad;			/* Zero padding byte */
	char d_type;   /* The filetype */
	char d_name[]; /* The filename (null terminated c string) */
};

typedef struct DIR_s
{
	filesystem_t *fs_info;
	offset_t      next_direntry_offset; // offset in file of next direntry
	ino_t         inode;
	blkcnt_t      blockpointerindex;
	size_t        lastentrysize;
	void *        filebuffer;
	struct dirent dirent;
} DIR;

#endif /* _SYS_DIRENT_H */