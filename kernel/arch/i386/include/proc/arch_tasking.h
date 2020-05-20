#ifndef ARCH_TASKING_H
#define ARCH_TASKING_H

#include <proc/tasking.h>
#include <mm/paging.h>

void arch_task_switch(task_t *next, task_t *prev);
/**
 * @brief      Architechture dependend jump to userspace function 
 */
void arch_jump_userspace(uint32_t eip, uint32_t argc, uint32_t argv);
void arch_spawn_task(uint32_t *esp, page_directory_t **dir);
void arch_flush_tlb();

#endif
