#include <proc/syscall.h>
#include <cpu/cpu.h>
#include <kernel.h>

/* all the imports for syscalls */
#include <proc/tasking.h>
#include <fs/vfs.h>
#include <yanix/exec.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <debug.h>

static void syscall_handler(registers_t *regs);

/* @todo: environment variable */
char *__env[1] = { 0 };
char **environ = __env;

void _exit() 
{
    debug_printk("Kill\n");
    send_sig(SIGKILL);
    asm volatile ("sti");
    kill_proc((task_t*)get_current_task());
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
    int ret = vfs_stat(file, st);
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

int sys_lseek(int fd, int offset, int mode)
{
    int ret = vfs_lseek(fd, offset, mode);
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

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    (void) (addr);
    (void) (length);
    (void) (prot);
    (void) (flags);
    (void) (fd);
    (void) (offset);
    return (void*) -ENOMEM;
}

int sys_munmap(void *addr, size_t length)
{
    (void) (addr);
    (void) (length);
    return -EINVAL;
}

pid_t sys_fork()
{
    printk("Fork has been called\n");
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

    return vfs_write_fd(fd, buf, amount);
}

int sys_close(int fd)
{
    return vfs_close_fd(fd);
    //return -1;
}

// @TODO: sbrk should be a lib func and not a syscall (should use a brk)

static const void *syscalls[] = {
    /* 0 */           0,
    /* 1 */           &_exit,           /* DONE */
    /* 2 */           &sys_fork,            /* DONE */
    /* 3 */           &vfs_read_fd,     /* DONE */
    /* 4 */           &sys_write,    /* DONE */
    /* 5 */           &vfs_open_fd,     /* DONE */
    /* 6 */           &sys_close,    /* DONE */
    /* 7 */           &wait,            /* NOT DONE */
    /* 8 */           &vfs_creat,       /* DONE */
    /* 9 */           &link,            /* NOT DONE */
    /* 10 */          &unlink,          /* NOT DONE */
    /* 11 */          &execve,          /* DONE */
    /* 12 */          &sys_kill,        /* DONE */
    /* 13 */          0,
    /* 14 */          &sys_lseek,       /* DONE */
    /* 15 */          &sbrk,            /* DONE */
    /* 16 */          &times,           /* NOT DONE */
    /* 17 */          &isatty,          /* NOT DONE */
    /* 18 */          &sys_stat,        /* DONE */
    /* 19 */          &signal,          /* NOT DONE */
    /* 20 */          &sys_readdir,     /* ???? */
    /* 21 */          &sys_getdents,    /* DONE */
    /* 22 */          &sys_chdir,       /* NOT DONE */
    /* 23 */          &sys_getcwd,      /* NOT DONE */
    /* 24 */          &sys_pipe,        /* NOT DONE */
    /* 25 */          &sys_mkdir,       /* NOT DONE */
    /* 26 */          &sys_fcntl,       /* NOT DONE */
    /* 27 */          0,
    /* 28 */          &sys_fstat,       /* DONE */
    /* 29 */          &sys_mmap,        /* NOT DONE */
    /* 30 */          &sys_munmap,      /* NOT DONE */
    /* 31 */          0,
    /* 32 */          0,
    /* 33 */          0,
    /* 34 */          0,
    /* 35 */          0,
    /* 36 */          0,
    /* 37 */          0,
    /* 38 */          0,
    /* 39 */          &getpid,          /* DONE */

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
    debug_printk(KERN_INFO "Syscall: %i with %x %x %x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);

    if (regs->eax >= NUMER_OF_SYSCALLS)
		return;

    void *location = (void*) syscalls[regs->eax];
    if (location == 0) 
    {
        printk(KERN_WARNING "SYSCALL %i NOT IMPLEMENTED\n", regs->eax);
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
   
   /* Return value is generally saved eax */
   regs->eax = ret;
}
