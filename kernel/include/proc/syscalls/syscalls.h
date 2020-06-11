#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include <fs/dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <yanix/system.h>

typedef void (*sighandler_t)(int);

struct siginfo;
struct sigset
{
	/* stubbed */
	int x;
};
struct sigaction
{
	void (*sa_handler)(int);
	void (*sa_sigaction)(int, struct siginfo *, void *);
	struct sigset sa_mask;
	int           sa_flags;
	void (*sa_restorer)(void);
};

struct timespec
{
	time_t tv_sec;
	time_t tv_nsec;
};

struct timeval
{
	time_t      tv_sec;
	suseconds_t tv_usec;
};

struct timezone
{
	int tz_minuteswest;
	int tz_dsttime;
};

struct rusage;
struct group;
struct timeval;
struct utsname;

void         sys_exit(int status);
int          sys_fstat(int fd, struct stat *st);
int          sys_stat(const char *file, struct stat *st);
int          sys_times(struct tms *buf);
int          sys_kill(pid_t pid, int sig);
int          sys_link(char *old, char *new);
int          sys_unlink(char *name);
int          sys_wait(int *status);
int          sys_lseek(int fd, int offset, int mode);
int          sys_isatty(int file);
sighandler_t sys_signal(int signum, sighandler_t handler);
int          sys_readdir(int fd, struct dirent *dirp, int count);
int          sys_getdents(int fd, struct dirent *dirp, int count);
int          sys_chdir(const char *path);
char *       sys_getcwd(char *s, int max);
int          sys_pipe(int pipefd[2]);
int          sys_mkdir(const char *path, mode_t mode);
int          sys_fcntl(int fd, int cmd, uintptr_t arguments);
void *       sys_mmap(void *addr, size_t length, int prot, int flags, int fd,
                      off_t offset);
int          sys_munmap(void *addr, size_t length);
pid_t        sys_fork();
ssize_t      sys_write(int fd, const void *buf, size_t amount);
int          sys_close(int fd);
int     sys_execve(const char *filename, const char **argv, char const **envp);
int     sys_getwd(char *buf);
int     sys_chown(char *pathname, uid_t owner, gid_t group);
int     sys_sysinfo(struct us_sysinfo *si);
int     sys_compatibility(int newmode);
void *  sys_sbrk(intptr_t incr);
int     sys_close(int fd);
int     sys_open(const char *path, int flags, int mode);
int     sys_creat(const char *path, int mode);
int     sys_getpid();
ssize_t sys_read(int fd, void *buf, size_t amount);

int    sys_dup(int oldfd);
int    sys_dup2(int oldfd, int newfd);
int    sys_sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);
uid_t  sys_geteuid();
uid_t  sys_getuid();
gid_t  sys_getgid();
mode_t sys_umask(mode_t mask);
pid_t  sys_getppid();
int    sys_lstat(const char *path, struct stat *buf);
int    sys_pipe2(int pipefd[2], int flags);
int    sys_ioctl(int fd, unsigned long request, char *argp);
int    sys_setuid(uid_t uid);
int    sys_seteuid(uid_t euid);
int    sys_getgroups(int size, gid_t *list);
int    sys_setgroups(size_t size, const gid_t *list);
int   sys_sigprocmask(int how, const struct sigset *set, struct sigset *oldset);
int   sys_getegid();
pid_t sys_wait3(int *status, int options, struct rusage *rusage);
int   sys_sigsuspend(const struct sigset *mask);
int   sys_sethostname(const char *name, size_t len);
int   sys_gethostname(char *name, size_t len);
unsigned int sys_alarm(unsigned int seconds);
int          sys_nanosleep(const struct timespec *req, struct timespec *rem);
int          sys_fchmod(int fd, mode_t mode);
int          sys_setgid(gid_t gid);
int          sys_setregid(gid_t real, gid_t effective);
int          sys_setreuid(uid_t real, uid_t effective);
int          sys_access(const char *pathname, int mode);
time_t       sys_time(time_t *tloc);
int          sys_requesterrno();
int          sys_mknod(const char *pathname, mode_t mode, dev_t dev);
int sys_select(int nfds, void *readfds, void *writefds, void *exceptfds,
               struct timeval *timeout);

void          sys_endgrent();
void          sys_setgrent();
struct group *sys_getgrent();
int           sys_uname(struct utsname *buf);
int           sys_gettimeofday(struct timeval *tv, struct timezone *tz);

#endif /* _SYSCALLS_H */
