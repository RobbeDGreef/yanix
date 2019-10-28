#include <proc/syscall.h>
#include <stdint.h>
#include <fs/dirent.h>

// @todo: this should use system calls

#if 0

/**
 * @brief      reads from given file descriptor
 *
 * @param[in]  fd     The file descriptor
 * @param[in]  buf    The buffer to read in
 * @param[in]  count  The amount of bytes to read
 *
 * @return     amount of bytes read on success and -1 if the filedescriptor could not find the file 
 */
int read(int fd, void *buf, size_t count)
{
	return sys_read(fd, buf, count);
}

/**
 * @brief      writes to given file descriptor
 *
 * @param[in]  fd     The file descriptor
 * @param[in]  buf    The buffer to write from
 * @param[in]  count  The amount of bytes to write
 *
 * @return     amount of bytes writen on success and -1 if the filedescriptor could not find the file 
 */
int write(int fd, const void *buf, size_t count)
{
	return sys_write(fd, buf, count);
}


/**
 * @brief      closes a filedescriptor
 *
 * @param[in]  fd    the filedescriptor
 *
 * @return     returns 0 at success and -1 when the filedescriptor could not be found
 */
int close(int fd)
{
	return sys_close(fd);
}


/**
 * @brief      opens a filedescriptor
 *
 * @param[in]  path  The path to the file to open
 * @param[in]  mode  The mode to open the file in
 *
 * @return     the filedescriptor
 */
int open(const char *path, int flags, int mode)
{
	return sys_open(path, flags, mode);
}


/**
 * @brief      opens a directory
 *
 * @param[in]  path  The path to the directory
 *
 * @return     a directory stream pointer
 */
DIR* opendir(const char *path)
{
	return sys_opendir(path);
}


/**
 * @brief      reads the next entry in a directory
 *
 * @param      dirp  The directory stream pointer
 *
 * @return     the next directory entry
 */
struct dirent *readdir(DIR *dirp)
{
	return sys_readdir(dirp);
}

#endif
