#include <cpu/gdt.h>
#include <debug.h>
#include <proc/tasking.h>
#include <kernel/stack.h>
#include <proc/threads.h>

/* Asm jump to userspace function */
extern void jmp_userspace(uint32_t eip, uint32_t stack, uint32_t argc,
                          uint32_t argv);
extern void do_task_switch(reg_t *previous_esp, reg_t next_esp, reg_t cr3, ...);

/**
 * @brief      Architechture dependend jump to userspace function
 */
void arch_jump_userspace(uint32_t eip, uint32_t stack, uint32_t argc,
                         uint32_t argv)
{
	struct thread *t = vec_thrds_get(get_current_task()->threads, 0);
	tss_set_kernel_stack(t->kernel_stack);
	jmp_userspace(eip, stack, argc, argv);
}

/**
 * @brief      Architechture dependend switch task function.
 *
 * @param      task  The task
 * @param[in]  ip    The instruction pointer
 * @param[in]  sp    The stack pointer
 * @param[in]  bp    The base pointer
 * @param[in]  cr3   The paging register
 * @param      tmp   The task
 */
void arch_task_switch(task_t *next, task_t *prev)
{
	// printk(KERN_DEBUG "next task stack: %x %x %x\n", next->kernel_stack,
	// next->esp, next->directory);

	struct thread *nt = vec_thrds_get(next->threads, 0);
	struct thread *pt = vec_thrds_get(prev->threads, 0);
	tss_set_kernel_stack(nt->kernel_stack);
	do_task_switch(&pt->stack, nt->stack, next->directory->physicalAddress,
	               next->directory);
}
