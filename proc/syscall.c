#include <proc/syscall.h>
#include <cpu/isr.h>

// all the imports for the syscalls
#include <proc/tasking.h>
#include <drivers/vfs/vfs.h>
#include <kernel/execute/exec.h>
#include <signal.h>
#include <errno.h>

static void syscall_handler(registers_t *regs);

// syscalls

// environment variable (@todo: )
char *__env[1] = { 0 };
char **environ = __env;


void _exit() 
{
    send_sig(SIGKILL);
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
    errno = ECHILD;
    return -1;
}

int lseek(const char* name, int flags, int mode)
{
    (void) (name);
    (void) (flags);
    (void) (mode);
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
    &lseek,         // 14 should be 8
    &sbrk,          // 15
    0,              // 16
    0,              // 17
    0,              // 18
    0,              // 19
    0,              // 20
    0,              // 21
    0,              // 22
    0,              // 23
    0,              // 24
    0,              // 25
    0,              // 26
    0,              // 27
    0,              // 28
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
    &getpid         // 39
};

const uint32_t num_syscalls = sizeof(syscalls) / sizeof(void*);

DEFN_SYSCALL0(0, 0);

/**
 * @brief      Initialzes the system calls
 */
void init_syscalls()
{
	register_interrupt_handler(0x80, &syscall_handler);
}

/**
 * @brief      The syscall handler (reroutes the syscall to kernel functions)
 *
 * @param      regs  The pushed registers
 */
static void syscall_handler(registers_t *regs){
	
    if (regs->eax >= num_syscalls)
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