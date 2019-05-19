#include <proc/syscall.h>
#include <cpu/isr.h>

static void syscall_handler(registers_t *regs);

// syscall setup

#include <drivers/video/videoText.h>

DEFN_SYSCALL1(print, 0, char*);

static void *syscalls[3] = {
	&print,
	&print_hex,
	&print_int
};

uint32_t num_syscalls = 3;

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
    print("syscall called\n");
	
    if (regs->eax >= num_syscalls)
		return;
	
    void *location = syscalls[regs->eax];

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