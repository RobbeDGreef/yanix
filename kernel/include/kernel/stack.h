#ifndef STACK_H
#define STACK_H

#include <mm/paging.h>
#include <mm/heap.h>
#include <stdint.h>

#define USER_STACK 0
#define KERNEL_STACK 1

#define KERNEL_MAIN_STACK 0xF00000
#define KERNEL_STACKHEAP_START 0x1000000
#define KERNEL_STACKHEAP_MAXSIZE 0x400000

#define USER_STACKHEAP_START 0x1400000
#define USER_STACKHEAP_MAXSIZE 0x400000

#define KERNEL_STACK_SIZE 0x4000
#define USER_STACK_SIZE 0x4000


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
void init_paging_stack(reg_t);
void set_user_stack();

void init_main_stack();
uintptr_t stack_alloc(int kernel, struct heap *heap);
void stack_free(int kernel, uintptr_t top);


#endif /* stack.h */