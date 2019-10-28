#include <cpu/gdt.h>
#include <proc/tasking.h>
#include <yanix/stack.h>

/* Asm jump to userspace function */
extern void jmp_userspace(uint32_t eip);
extern void task_switch(uint32_t ip, uint32_t sp, uint32_t bp, uint32_t cr3);

/* Running task reference */
extern volatile task_t *g_runningtask;

/**
 * @brief      Architechture dependend jump to userspace function 
 */
void arch_jump_userspace(uint32_t eip)
{
	tss_set_kernel_stack(g_runningtask->kernel_stack + KERNEL_STACK_SIZE);
	jmp_userspace(eip);
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