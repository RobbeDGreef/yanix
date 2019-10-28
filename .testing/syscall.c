#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#define SYSCALL_0(num) 				(  int ret; \
									   asm voaltile ("int $0x80" : "=a" (ret) : "0" (num)); \
									   return ret;) 			
	
#define SYSCALL_1(num, P1)			(  int ret; \
									   asm volatile ("int $0x80" : "=a" (ret) : "0" (num); "b" ((int P1))); \
									   return ret; )
	
#define SYSCALL_2(num, P1, P2) 		(  int ret; \
									   asm volatile ("int $0x80" : "=a" (ret) : "0" (num), "b" ((int) P1), "c" ((int) P2)); \ 
									   return ret; )

#define SYSCALL_3(num, P1, P2, P3)	(  int ret; \
								   	   asm volatile ("int $0x80" : "=a" (ret) : "0" (num), "b" ((int) P1), "c" ((int) P2), "d") ((int P3)); \ 
								   	   return ret; )



void _exit()
{
	// _exit syscall
	SYSCALL_0(1); 	// _exit function is first syscall
}

int close(int fd)
{
	SYSCALL_1(6, fd); // vfs_close_raw function is functioncall number 6
}

int open(const char *name, int flags, int mode)
{
	SYSCALL_3(5, name, flags, mode);
}

int read(int file, char *ptr, int len)
{
	SYSCALL_3(3, file, ptr, len);
}

int write(int file, char *ptr, int len)
{
	SYSCALL_3(4, file, ptr, len);
}


extern char **environ; // environment variables

int execve(char *name, char **argv, char **env)
{
	SYSCALL_3(11, name, argv, env);
}

int fork()
{
	SYSCALL_0(2);
}

int fstat(int file, struct stat *st)
{
	// @todo: implement fstat function and syscall
	st->st_mode = S_IFCHR;
  	return 0;
}

int getpid()
{
	SYSCALL_0(39);
}

int isatty(int fd)
{
	if (fd <= 3) {
		return 1;
	} 
	return 0;
}

caddr_t sbrk(int incr) 
{
	SYSCALL_1(15, incr);
}

int kill(int pid, int sig) 
{
	SYSCALL_2(12, pid, sig);
}

int times(struct tms *buf)
{
	return -1;
}

int link(char *old, char *new)
{
	SYSCALL_2(9, old, new);
}

int unlink(char *name)
{
	SYSCALL_1(13, name);
}

int wait(int *status)
{
	SYSCALL_1(7, status);
}

int stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int lseek(int file, int ptr, int dir)
{
	SYSCALL_3(14, file, ptr, dir);
}