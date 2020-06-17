#ifndef _YANIX_SYS_DIRENT_H
#define _YANIX_SYS_DIRENT_H

/* Yanix customized code */

/**
 * @brief      dirent structure according to POSIX standards
 */
struct dirent
{
	unsigned long  d_ino;    /* The inode number */
	unsigned long  d_off;    /* The offset to the next dirent */
	unsigned short d_reclen; /* Length of this dirent structure in bytes */

	char d_type;   /* The filetype */
	char d_name[]; /* The filename (null terminated c string) */
};

typedef struct
{
	int   dd_fd;  /* Directory file descriptor */
	int   dd_loc; /* Position in buffer in bytes */
	int   dd_seek;
	char *dd_buf;  /* Buffer */
	int   dd_len;  /* Buffer length in bytes*/
	int   dd_size; /* Amount of data in buffer in bytes*/
} DIR;

#endif /* _YANIX_SYS_DIRENT_H */
