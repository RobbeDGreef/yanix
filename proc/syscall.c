#include <proc/syscall.h>
#include <cpu/isr.h>

// all the imports for the syscalls
#include <proc/tasking.h>
#include <drivers/vfs/vfs.h>
#include <kernel/execute/exec.h>

static void syscall_handler(registers_t *regs);

// syscall setup

static const void *syscalls[] = {
	0,              // int sys_setup(void)
    &exit_proc,     // int exit_proc(int status)
    &fork,          // int fork()
    &vfs_read_raw,  // ssize_t read(unsigned int fd, char *buf, size_t count)
    &vfs_write_raw, // ssize_t write(unsigned int fd, const char *buf, size_t count)
    &vfs_open_raw,  // int open(const char *filename, int flags, int mode)
    &vfs_close_raw, // void close(unsigned int fd)
    0,              // int waitpid(pid_t pid, unsigned int *stat_addr, int options)
    0,              // int creat(const char *pathname, int mode)
    0,              // int link(const char *oldname, const char *newname)
    0,              // int unlink(const char *pathname)
    &execve         // int execve(struct pt_regs, regs)
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