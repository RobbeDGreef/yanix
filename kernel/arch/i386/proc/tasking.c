#include <cpu/gdt.h>
#include <proc/tasking.h>
#include <yanix/stack.h>
#include <debug.h>
/* Asm jump to userspace function */
extern void jmp_userspace(uint32_t eip, uint32_t argc, uint32_t argv);
extern void task_switch(uint32_t ip, uint32_t sp, uint32_t bp, uint32_t cr3);

/**
 * @brief      Architechture dependend jump to userspace function 
 */
void arch_jump_userspace(uint32_t eip, uint32_t argc, uint32_t argv)
{
	tss_set_kernel_stack(get_current_task()->kernel_stack);
	jmp_userspace(eip, argc, argv);
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
void arch_task_switch(task_t *task, unsigned long ip, unsigned long sp, unsigned long bp, unsigned long cr3)
{
	tss_set_kernel_stack(task->kernel_stack+KERNEL_STACK_SIZE);
	task_switch(ip, sp, bp, cr3);
}