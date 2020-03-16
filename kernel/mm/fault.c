#include <mm/paging.h>
#include <mm/heap.h>
#include <sys/types.h>
#include <cpu/cpu.h>
#include <signal.h>

#include <errno.h>
#include <debug.h>

extern void debug_handler(registers_t *);

void zero_page(offset_t addr, page_directory_t *pdir, int user)
{
	alloc_frame(get_page(addr, 1, pdir), user ? 0 : 1, user);
	/* @todo: actually zero the page out, I don't think it really needs to happen but anyway */
}

void error_message(offset_t addr, registers_t *regs, int present, int rw, int us, int reserved)
{
	if (get_current_task()->ring != 3)
	{		
		/* this is a kernel task */
		printk("Page fault: \n");
		if (present)  {printk("was not present, ");}
		if (rw) 	  {printk("read write, "); errno = -EPERM;}
		if (us) 	  {printk("user-mode, ");  errno = -EACCES;}
		if (reserved) {printk("reserved, ");   errno = -EACCES;}
		printk(" at 0x%x\n", addr);

		debug_handler(regs);
		for(;;);
	}
	else 
	{
		/* this is a user task */
		printk("Segmentation fault, faulting address: 0x%08x\n", addr);
		if (present)  printk("was not present, ");
		if (rw) 	  printk("read write, ");
		if (us) 	  printk("user-mode, ");
		if (reserved) printk("reserved, ");
		debug_handler(regs);
		send_sig(SIGKILL);
		for(;;);
	}
}

void page_fault(registers_t *regs)
{
	offset_t faulting_address;
	asm volatile("mov %%cr2, %0":"=r"(faulting_address));

	/* 0 means that the page was not present in regs->err_code & 0x1 */
	int not_present = !(regs->err_code & 0x1);	/* Whether the page was present */
	int rw = regs->err_code & 0x2;			/* whether it was a write operation or not */
	int us = regs->err_code & 0x4;			/* processor in ring 3 */
	int reserved = regs->err_code & 0x8;	/* Overwritten CPU-reserved bits of a page entry */

	/* Check wheter it was part of a heap and should be zero'd out */
	struct heap *heap = is_addr_in_heap(faulting_address);
	
	if (not_present)
	{
		if (heap)
			zero_page(faulting_address, get_kernel_dir(), heap->usermode);

		/* Check wheter it was part of the user space program running
		 * with the program break and zero it out */
		else if (faulting_address >= get_current_task()->program_start && faulting_address <= get_current_task()->program_break)
			zero_page(faulting_address, get_current_dir(), get_current_task()->ring);

		/* Otherwise call the error message */
		else
			error_message(faulting_address, regs, not_present, rw, us, reserved);
		
	} 
	else
	{
		error_message(faulting_address, regs, not_present, rw, us, reserved);
	}
}