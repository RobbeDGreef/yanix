#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <mm/paging.h>

#define DISIRED_STACK_LOCATION 	0x1000000	// 0x185000 end of kernelpreheap and all 
#define STACK_SIZE				0x1000
#define STACK_LOCATION			0x9000
#define KERNEL_STACK_SIZE 		0x1000

extern uint32_t g_initial_esp;

/**
 * @brief      Saves the stack location
 *
 * @param[in]  stack_location  The stack location
 */
void init_stack(uint32_t stack_location);

/**
 * @brief      Initializes the stack with paging and bind it to the disired virtual address
 */
void init_paging_stack();

#endif /* stack.h */