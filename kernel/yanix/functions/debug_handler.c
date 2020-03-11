#include <kernel.h>
#include <cpu/cpu.h>
#include <debug.h>

#include <mm/paging.h>
extern page_directory_t *g_current_directory;

void debug_handler(registers_t *regs)
{


	printk("============ DEBUG HANDLER CALLED ============\n");
	
	if (regs == 0)
	{
		printk("No register data was provided\n");
		goto stacktrace;
	}

	printk("Interrupt number: %i (0x%x) and error number %i (0x%x)\n", regs->int_no, regs->int_no, regs->err_code, regs->err_code);
	printk("Register dump: \n");
	printk("EAX: %08x EBX: %08x ECX: %08x EDX: %08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
	printk("DS:  %08x CS:  %08x SS:  %08x EF:  %08x\n", regs->ds, regs->cs, regs->ss, regs->eflags);
	printk("ESP: %08x EBP: %08x ESI: %08x EDI: %08x\n", regs->esp, regs->ebp, regs->esi, regs->edi);
	printk("EIP: %08x CR3: %08x\n", regs->eip, g_current_directory->physicalAddress);

stacktrace:;
	printk("----------------- STACKTRACE -----------------\n");
	dump_stacktrace();
	printk("============       DEBUG END      ============\n");

	for(;;);
}
