#ifndef STACK_H
#define STACK_H

#include <mm/paging.h>
#include <stdint.h>

#define DISIRED_STACK_LOCATION   0x1000000
#define DISIRED_USER_STACK_LOC   0x2000000
#define DISIRED_KERNEL_STACK_LOC 0x3000000
#define STACK_SIZE               0x1000
#define STACK_LOCATION           0x9000
#define KERNEL_STACK_SIZE        0x8000
#define USER_STACK_SIZE          0x4000 // 16Kib

extern uint32_t g_initial_esp;

/**
 * @brief      Saves the stack location
 *
 * @param[in]  stack_location  The stack location
 */
void init_stack(uint32_t stack_location);

/**
 * @brief      Initializes the stack with paging and bind it to the disired
 * virtual address
 */
void init_paging_stack();
void set_user_stack();

#endif /* stack.h */