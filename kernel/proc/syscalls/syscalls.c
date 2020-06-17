#include <cpu/cpu.h>
#include <kernel.h>
#include <proc/syscall.h>

/* all the imports for syscalls */
#include <debug.h>
#include <errno.h>
#include <fcntl.h>
#include <fs/pipe.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <proc/sched.h>
#include <proc/syscalls/syscalls.h>
#include <proc/tasking.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <kernel/exec.h>
#include <kernel/system.h>
#include <yanix/sys/ioctl.h>

void sys_exit(int status)
{
	debug_printk("Kill %i\n", status);
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

int sys_stat(const char *file, struct stat *st)
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
	printk("sys kill\n");
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

sighandler_t sys_signal(int signum, sighandler_t handler)
{
	(void) (signum);
	(void) (handler);
	signal(signum, handler);
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
	char *buf = vfs_fullpath((char *) path);

	if (vfs_find_path(buf))
	{
		kfree(get_current_task()->cwd);
		get_current_task()->cwd = buf;

		return 0;
	}
	return -ENOENT;
}

int sys_requesterrno()
{
	debug_printk("Requested errno\n\n\n");
	return errno;
}

char *sys_getcwd(char *s, int max)
{
	char *cwd = get_current_task()->cwd;
	int   len = strlen(cwd) + 1;

	if (s == NULL)
	{
		if (max && max < len)
		{
			errno = ERANGE;
			return NULL;
		}
		else if (max)
		{
			len = max;
		}

		s = kmalloc_user(len);
	}

	strcpy_s(s, cwd, len);
	return s;
}

int sys_pipe(int pipefd[2])
{
	return pipe(pipefd);
}

int sys_mkdir(const char *path, mode_t mode)
{
	(void) (path);
	(void) (mode);
	return -1;
}

int sys_fcntl(int fd, int cmd, uintptr_t arg)
{
	switch (cmd)
	{
	case F_DUPFD:
		return dup_filedescriptor(fd, arg);
	case F_SETFD:
		return setflags_filedescriptor(fd, arg);
	}

	return 0;
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

ssize_t sys_write(int fd, const void *buf, size_t amount)
{
	if (fd == 2 && amount > 0xC00)
	{
		debug_printk("Error, stderr lock for newlib bug called, write did not "
		             "go through\n");
		return amount;
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
	execve_user(filename, argv, envp);
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

int sys_sysinfo(struct us_sysinfo *si)
{
	memset(si, 0, sizeof(struct us_sysinfo));

	si->uptime    = timer_secs_since_boot();
	si->totalram  = g_system.totalram;
	si->freeram   = phys_freeram_amount();
	si->sharedram = sharedram_amount();
	si->bufferram = bufferram_amount();
	si->totalswap = g_system.totalswap;
	si->freeswap  = phys_freeswap_amount();
	si->procs     = get_proccount();

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
		page_t *page = get_page(i, 1, get_current_task()->directory);
		if (flagforce_alloc_frame(page, kernel, 1))
			return (void *) -1;
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
	if ((int) path == -1)
		return -ENOENT;

	int ret = vfs_open_fd(path, flags, mode);
	if (ret == -1)
		return -errno;

	debug_printk("open: %s as %i\n", path, ret);
	return ret;
}

ssize_t sys_read(int fd, void *buf, size_t amount)
{
	int ret = vfs_read_fd(fd, buf, amount);
	if (ret == -1)
		return -errno;

	return ret;
}

uid_t sys_getuid()
{
	return get_current_task()->uid;
}

uid_t sys_geteuid()
{
	return get_current_task()->euid;
}

int sys_dup2(int oldfd, int newfd)
{
	int ret = dup2_filedescriptor(oldfd, newfd);

	if (ret == -1)
		return -errno;

	return ret;
}

int sys_dup(int oldfd)
{
	return dup_filedescriptor(oldfd, -1);
}

struct sigaction;
int sys_sigaction(int signum, const struct sigaction *act,
                  struct sigaction *oldact)
{
	return -1;
}

mode_t sys_umask(mode_t mask)
{
	mode_t umask = get_current_task()->umask;
	get_current_task()->umask &= 0777;
	return umask;
}

pid_t sys_getppid()
{
	return get_current_task()->parent;
}

/**
 * @todo: this is not the correct implementation
 */
int sys_lstat(const char *path, struct stat *buf)
{
	return sys_stat(path, buf);
}

int sys_pipe2(int pipefd[2], int flags)
{
	return -1;
}

gid_t sys_getgid()
{
	return get_current_task()->gid;
}

int sys_ioctl(int fd, unsigned long request, char *argp)
{
	return ioctl(fd, request, argp);
}

/**
 * @todo: these two are like, the biggest possible form
 * 		  of security hazzards you can have and are just stubs.
 */
int sys_setuid(uid_t uid)
{
	get_current_task()->uid = uid;
	return 0;
}

int sys_seteuid(uid_t euid)
{
	get_current_task()->euid = euid;
	return 0;
}

int sys_getgroups(int size, gid_t *list)
{
	return -1;
}

int sys_setgroups(size_t size, const gid_t *list)
{
	return -1;
}

struct sigset;
int sys_sigprocmask(int how, const struct sigset *set, struct sigset *oldset)
{
	return -1;
}

int sys_getegid()
{
	return get_current_task()->egid;
}

struct rusage;
/* @todo: this is a stub */
pid_t sys_wait3(int *status, int options, struct rusage *rusage)
{
	return sys_wait(status);
}

int sys_sigsuspend(const struct sigset *mask)
{
	return -1;
}

int sys_gethostname(char *name, size_t len)
{
	strcpy_s(name, g_system.hostname, len);
	return 0;
}

int sys_sethostname(const char *name, size_t len)
{
	kfree(g_system.hostname);
	g_system.hostname = strdup(name);
	return 0;
}

unsigned int sys_alarm(unsigned int seconds)
{
	return -1;
}

int sys_nanosleep(const struct timespec *req, struct timespec *rem)
{
	int time = req->tv_sec * 1000;
	time += (uint32_t) req->tv_nsec / 1000;
	sleep(time);
	return 0;
}

int sys_fchmod(int fd, mode_t mode)
{
	return -1;
}

int sys_setgid(gid_t gid)
{
	get_current_task()->gid = gid;
	return 0;
}

int sys_setregid(gid_t real, gid_t effective)
{
	get_current_task()->gid  = real;
	get_current_task()->egid = effective;
	return 0;
}

int sys_setreuid(uid_t real, uid_t effective)
{
	get_current_task()->uid  = real;
	get_current_task()->euid = effective;
	return 0;
}

int sys_access(const char *pathname, int mode)
{
	return -1;
}

time_t sys_time(time_t *tloc)
{
	return (time_t) -1;
}

int sys_mknod(const char *pathname, mode_t mode, dev_t dev)
{
	if (mode & S_IFIFO)
		return mkfifo(pathname);

	return -1;
}

struct group *sys_getgrent()
{
	return NULL;
}

void sys_setgrent()
{
}

void sys_endgrent()
{
}

int sys_select(int nfds, void *readfds, void *writefds, void *exceptfds,
               struct timeval *timeout)
{
	return -1;
}

#define _UTSNAME_ENTRY_LEN 65
struct utsname
{
	char sysname[_UTSNAME_ENTRY_LEN];
	char nodename[_UTSNAME_ENTRY_LEN];
	char release[_UTSNAME_ENTRY_LEN];
	char version[_UTSNAME_ENTRY_LEN];
	char machine[_UTSNAME_ENTRY_LEN];
};

int sys_uname(struct utsname *buf)
{
	int sysname_len  = strlen(g_system.sysname) + 1;
	int nodename_len = strlen(g_system.sysname) + 1;
	int release_len  = strlen(g_system.release) + 1;
	int version_len  = strlen(g_system.version) + 1;
	int machine_len  = strlen(g_system.machine) + 1;

	memcpy(buf->sysname, g_system.sysname, sysname_len);
	memcpy(buf->nodename, g_system.sysname, nodename_len);
	memcpy(buf->release, g_system.release, release_len);
	memcpy(buf->version, g_system.version, version_len);
	memcpy(buf->machine, g_system.machine, machine_len);

	return 0;
}

int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	if (tv == NULL)
		return -1;

	time_since_boot((time_t *) &tv->tv_sec, (time_t *) &tv->tv_usec);
	return 0;
}

int sys_socket(int domain, int type, int protocol)
{
	int ret = socket(domain, type, protocol);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_bind(int soc, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = sock_bind(soc, addr, addrlen);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_listen(int soc, int backlog)
{
	int ret = sock_listen(soc, backlog);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_accept(int soc, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret = sock_accept(soc, addr, addrlen);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_send(int soc, const void *buf, size_t len, int flags)
{
	int ret = sock_send(soc, buf, len, flags);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_recv(int soc, void *buf, size_t len, int flags)
{
	int ret = sock_recv(soc, buf, len, flags);
	if (ret == -1)
		return -errno;

	return ret;
}

int sys_connect(int soc, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = sock_connect(soc, addr, addrlen);
	if (ret == -1)
		return -errno;

	return ret;
}