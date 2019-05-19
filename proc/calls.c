#include <errno.h>
#include <sys/types.h>

#include <proc/tasking.h>
#include <lib/stdio/stdio.h>

int sys_setup()
{
	return -ENOSYS;
}

int sys_exit(int status)
{
	(void) (status);
	return -ENOSYS;
}

int sys_fork()
{
	return fork();
}

ssize_t sys_read(unsigned int fd, char *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t sys_write(unsigned int fd, const char *buf, size_t count)
{
	return write(fd, buf, count);
}

int sys_open(const char *filename, int flags, int mode)
{
	return open(filename, flags, mode);
}

int sys_close(unsigned int fd)
{
	return close(fd);
}

int sys_waitpid(pid_t pid, unsigned int *start_addr, int options)
{
	(void) (pid);
	(void) (start_addr);
	(void) (options);
	return -ENOSYS;
}

#if 0
int sys_creat(const char *pathname, int mode)
{

}
#endif