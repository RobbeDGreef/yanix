#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include <fs/dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
typedef void (*sighandler_t)(int);

void         sys_exit();
int          sys_fstat(int fd, struct stat *st);
int          sys_stat(char *file, struct stat *st);
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
int          sys_getcwd(char *s, int max);
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
int     sys_sysinfo();
int     sys_compatibility(int newmode);
void *  sys_sbrk(intptr_t incr);
int     sys_close(int fd);
int     sys_open(const char *path, int flags, int mode);
int     sys_creat(const char *path, int mode);
int     sys_getpid();
ssize_t sys_read(int fd, void *buf, size_t amount);

#endif /* _SYSCALLS_H */
