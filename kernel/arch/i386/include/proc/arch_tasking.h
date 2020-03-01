#ifndef ARCH_TASKING_H
#define ARCH_TASKING_H

#include <proc/tasking.h>


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
void arch_task_switch(task_t *task, unsigned long ip, unsigned long sp, unsigned long bp, unsigned long cr3);

/**
 * @brief      Architechture dependend jump to userspace function 
 */
void arch_jump_userspace(uint32_t eip, uint32_t argc, uint32_t argv);


#endif
