#include <cpu/cpu.h>
#include <kernel.h>
#include <proc/syscall.h>

/* all the imports for syscalls */
#include <debug.h>
#include <errno.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <proc/sched.h>
#include <proc/tasking.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <yanix/exec.h>

void sys_exit()
{
	debug_printk("Kill\n");
	send_sig(SIGKILL);
	kill_proc((task_t *) get_current_task());
	/* This is a safety to handle the very slight change of there not being any
	 * task to yield to (i might change this in the future) */
	for (;;)
		;
}

int sys_fstat(int fd, struct stat *st)
{
	int ret = vfs_fstat(fd, st);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_stat(char *file, struct stat *st)
{
	int ret = vfs_stat(file, st);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_times(struct tms *buf)
{
	(void) (buf);
	return -1;
}

int sys_kill(pid_t pid, int sig)
{
	return send_pid_sig(pid, sig);
}

int sys_link(char *old, char *new)
{
	(void) (old);
	(void) (new);
	errno = EMLINK;
	return -1;
}

int sys_unlink(char *name)
{
	(void) (name);
	errno = ENOENT;
	return -1;
}

int sys_wait(int *status)
{
	if (!task_wait(status))
		return 0;

	else
		return -errno;
}

int sys_lseek(int fd, int offset, int mode)
{
	int ret = vfs_lseek(fd, offset, mode);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_isatty(int file)
{
	(void) (file);
	return 1;
}

typedef void (*sighandler_t)(int);

sighandler_t sys_signal(int signum, sighandler_t handler)
{
	(void) (signum);
	(void) (handler);
	return 0;
}

int sys_readdir(int fd, struct dirent *dirp, int count)
{
	/* Depricated so never implemented */
	(void) (fd);
	(void) (dirp);
	(void) (count);
	return -1;
}

int getdents(int fd, struct dirent *dirp, int count);

int sys_getdents(int fd, struct dirent *dirp, int count)
{
	int size = getdents(fd, dirp, count);

	if (size == -1)
		return -errno;

	return size;
}

int sys_chdir(const char *path)
{
	if (vfs_find_path(path))
	{
		kfree(get_current_task()->cwd);
		get_current_task()->cwd = strdup(path);

		return 0;
	}
	return -ENOENT;
}

int sys_getcwd(char *s, int max)
{
	strcpy_s(s, get_current_task()->cwd, max);
	return 0;
}

int sys_pipe(int pipefd[2])
{
	(void) (pipefd);
	return -1;
}

int sys_mkdir(const char *path, mode_t mode)
{
	(void) (path);
	(void) (mode);
	return -1;
}

int sys_fcntl(int fd, int cmd, uintptr_t arguments)
{
	(void) (fd);
	(void) (cmd);
	(void) (arguments);
	printk(KERN_DEBUG "AAAAAA\n");
	return -1;
}

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd,
               off_t offset)
{
	(void) (addr);
	(void) (length);
	(void) (prot);
	(void) (flags);
	(void) (fd);
	(void) (offset);
	return (void *) -ENOMEM;
}

int sys_munmap(void *addr, size_t length)
{
	(void) (addr);
	(void) (length);
	return -EINVAL;
}

pid_t sys_fork()
{
	return fork();
}

#include <yanix/tty_dev.h>

ssize_t sys_write(int fd, const void *buf, size_t amount)
{
	if (fd == 1)
	{
		tty_set_color(TTY_LIGHT_BLUE);
		ssize_t ret = vfs_write_fd(fd, buf, amount);
		tty_set_color(TTY_WHITE);
		return ret;
	}

	if (fd == 2 && amount > 0xC00)
	{
		debug_printk("Error, stderr lock for newlib bug called, write did not "
		             "go through\n");
		return 0;
	}

	int ret;
	if ((ret = vfs_write_fd(fd, buf, amount)) == -1)
		return -errno;

	return ret;
}

int sys_close(int fd)
{
	// We don't want to close the in, out and err
	if (fd < 3)
		return -1;

	int ret;
	if ((ret = vfs_close_fd(fd)) == -1)
		return -errno;

	return ret;
}

int sys_execve(const char *filename, const char **argv, char const **envp)
{
	execve(filename, argv, envp);
	return -errno;
}

int sys_getwd(char *buf)
{
	return 0;
}

int sys_chown(char *pathname, uid_t owner, gid_t group)
{
	return 0;
}

int sys_sysinfo()
{
	return 0;
}

int sys_compatibility(int newmode)
{
	return 0;
}

void *sys_sbrk(intptr_t incr)
{
	int kernel = 1;
	if (get_current_task()->ring == 3)
		kernel = 0;

	for (uint32_t i = get_current_task()->program_break;
	     i < get_current_task()->program_break + incr;
	     i += 0x1000)
	{
		// this will alocate a frame if the frame has not already been set
		if (alloc_frame(get_page(i, 1, get_current_task()->directory), kernel,
		                kernel ? 0 : 1)
		    == -2)
		{
			return (void *) -1;
		}
	}
	get_current_task()->program_break += incr;
	return (void *) (get_current_task()->program_break - incr);
}

int sys_getpid()
{
	return getpid();
}

int sys_creat(const char *path, int mode)
{
	int ret = vfs_creat(path, mode);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_open(const char *path, int flags, int mode)
{
	int ret = vfs_open_fd(path, flags, mode);
	if (ret == -1)
		return -errno;

	return ret;
}

ssize_t sys_read(int fd, void *buf, size_t amount)
{
	int ret = vfs_read_fd(fd, buf, amount);
	if (ret == -1)
		return -errno;

	return ret;
}