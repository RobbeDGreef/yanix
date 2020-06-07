#include <cpu/cpu.h>
#include <cpu/interrupts.h>
#include <debug.h>
#include <proc/syscalls/compat.h>

void **current_syscall_list = NULL;
int    syscall_amount       = 0;

static void syscall_handler(registers_t *regs)
{
	if (regs->eax != 4)
		debug_printk(KERN_DEBUG "syscall: %i '%i' '%i' '%i'\n", regs->eax,
		             regs->ebx, regs->ecx, regs->edx);
	if (regs->eax >= (uint) syscall_amount)
		return;

	void *syscall = (void *) current_syscall_list[regs->eax];
	if (syscall == 0)
	{
		printk(KERN_WARNING "SYSCALL %i NOT IMPLEMENTED\n", regs->eax);
		return;
	}

	end_of_interrupt();
	enable_interrupts();

	int ret;
	asm volatile(" \
    	push %1; \
    	push %2; \
     	push %3; \
     	push %4; \
     	push %5; \
     	call *%6; \
     	add $20, %%esp; \
        "
	             : "=a"(ret)
	             : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx),
	               "r"(regs->ecx), "r"(regs->ebx), "r"(syscall));

	/* Return value is generally saved eax */
	regs->eax = ret;
}

int switch_syscalls(int newmode)
{
	switch (newmode)
	{
	case COMPAT_LINUX:
		current_syscall_list = syscalls_linux;
		syscall_amount       = syscall_linux_amount;
		return 0;

	case COMPAT_YANIX:
		current_syscall_list = syscalls_yanix;
		syscall_amount       = syscall_yanix_amount;
		return 0;

	default:
		return -1;
	}
}

int init_syscalls()
{
	current_syscall_list = syscalls_yanix;
	syscall_amount       = syscall_yanix_amount;
	arch_register_interrupt_handler(0x80, &syscall_handler);
	return 0;
}