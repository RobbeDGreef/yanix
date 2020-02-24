#include <proc/syscall.h>
#include <cpu/cpu.h>
#include <kernel.h>

// all the imports for the syscalls
#include <proc/tasking.h>
#include <fs/vfs.h>
#include <yanix/exec.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <debug.h>

static void syscall_handler(registers_t *regs);


// syscalls

// environment variable (@todo: )
char *__env[1] = { 0 };
char **environ = __env;

void _exit() 
{
    send_sig(SIGKILL);
    kill_proc((task_t*)get_current_task());
    task_yield();

    /* This is a safety to handle the very slight change of there not being any task to yield to (i might change this in the future) */
    for(;;);
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
    int ret = vfs_stat(fd, st);
    if (ret == -1)
        return -errno;
    
    return ret;
}

int times(struct tms *buf)
{
    (void) (buf);
    return -1;
}

int sys_kill(pid_t pid, int sig)
{
    return send_pid_sig(pid, sig);
}

int link(char *old, char *new)
{
    (void) (old);
    (void) (new);
    errno = EMLINK;
    return -1;
}

int unlink(char *name)
{
    (void) (name);
    errno = ENOENT;
    return -1;
}

int wait(int *status)
{
    (void) (status);
    errno = ECHILD;
    return -1;
}

int sys_lseek(const char* name, int offset, int mode)
{
    int ret = vfs_lseek(name, offset, mode);
    if (ret == -1)
        return -errno;
    
    return ret;
}

int isatty(int file)
{
    (void) (file);
    return 1;
}

typedef void (*sighandler_t) (int);

sighandler_t signal(int signum, sighandler_t handler)
{
    (void) (signum);
    (void) (handler);
    return 0;
}

int sys_readdir(int fd, struct dirent* dirp, int count)
{
    /* Depricated so never implemented */
    (void) (fd);
    (void) (dirp);
    (void) (count);
    return -1;
}


int getdents(int fd, struct dirent* dirp, int count);

int sys_getdents(int fd, struct dirent* dirp, int count)
{
    int size = getdents(fd, dirp, count);

    if (size == -1)
        return -errno;

    return size;
}

int sys_chdir(const char *path)
{
    (void) (path);
    return -1;
}

int sys_getcwd()
{
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

// @todo: ok so im done with placing the syscalls in the right order FIX SYSCALL ORDER (linux)
// @TODO: sbrk should be a lib func and not a syscall (should use a brk)

static const void *syscalls[] = {
	0,              // 0  int sys_setup(void)
    &_exit,         // 1  int exit_proc(int status)
    &fork,          // 2  int fork()
    &vfs_read_fd,   // 3  ssize_t read(unsigned int fd, char *buf, size_t count)
    &vfs_write_fd,  // 4  ssize_t write(unsigned int fd, const char *buf, size_t count)
    &vfs_open_fd,   // 5  int open(const char *filename, int flags, int mode)
    &vfs_close_fd,  // 6  void close(unsigned int fd)
    &wait,          // 7  int waitpid(pid_t pid, unsigned int *stat_addr, int options)
    &vfs_creat,     // 8  int creat(const char *pathname, int mode)
    &link,          // 9  int link(const char *oldname, const char *newname)
    &unlink,        // 10 int unlink(const char *pathname)
    &execve,        // 11 int execve(struct pt_regs, regs)
    &sys_kill,      // 12 int kill(int pid, int sig) @TODO: THIS SHOULD BE SYSCALL 62
    &link,          // 13 should be 86
    &sys_lseek,         // 14 should be 8
    &sbrk,          // 15
    &times,         // 16
    &isatty,        // 17
    &sys_stat,          // 18
    &signal,        // 19
    &sys_readdir,   // 20
    &sys_getdents,  // 21
    &sys_chdir,     // 22
    &sys_getcwd,    // 23
    &sys_pipe,      // 24
    &sys_mkdir,     // 25
    &sys_fcntl,     // 26
    0,              // 27
    &sys_fstat,         // 28
    0,              // 29
    0,              // 30
    0,              // 31
    0,              // 32
    0,              // 33
    0,              // 34
    0,              // 35
    0,              // 36
    0,              // 37
    0,              // 38
    &getpid,        // 39

};

#define NUMER_OF_SYSCALLS sizeof(syscalls) / sizeof(syscalls[0])

/**
 * @brief      Initialzes the system calls
 */
int init_syscalls()
{
	arch_register_interrupt_handler(0x80, &syscall_handler);
    return 0;
}

/**
 * @brief      The syscall handler (reroutes the syscall to kernel functions)
 *
 * @param      regs  The pushed registers
 */
static void syscall_handler(registers_t *regs){
    //printk("Syscall: %i with %i %i %i\n", regs->eax, regs->ebx, regs->ecx, regs->edx);

    if (regs->eax >= NUMER_OF_SYSCALLS)
		return;

    void *location = (void*) syscalls[regs->eax];
    if (location == 0) {
        return;
    }

	int ret;
	asm volatile (" \
    	push %1; \
    	push %2; \
     	push %3; \
     	push %4; \
     	push %5; \
     	call *%6; \
     	pop %%ebx; \
     	pop %%ebx; \
     	pop %%ebx; \
     	pop %%ebx; \
     	pop %%ebx; \
   		" : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
   regs->eax = ret;
}
//0x80482b2