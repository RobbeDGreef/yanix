#ifndef STACK_H
#define STACK_H

#include <mm/paging.h>
#include <mm/heap.h>
#include <stdint.h>

#define USER_STACK 0
#define KERNEL_STACK 1

#define KERNEL_STACK_SIZE 0x5000
#define USER_STACK_SIZE 0x4000

#define KERNEL_MAIN_STACK 0xF00000
#define KERNEL_MAIN_KSTACK KERNEL_MAIN_STACK - KERNEL_STACK_SIZE

#define KERNEL_STACKHEAP_START 0x1000000
#define KERNEL_STACKHEAP_MAXSIZE 0x400000

#define USER_STACKHEAP_START 0x1400000
#define USER_STACKHEAP_MAXSIZE 0x400000


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

uintptr_t stack_alloc(int kernel, struct heap *heap);
void stack_free(int kernel, uintptr_t top);

inline void __attribute__((always_inline)) init_main_stack()
{
	extern struct heap *g_kernel_stackheap;
	g_kernel_stackheap = create_heap(NULL, KERNEL_STACKHEAP_START, KERNEL_STACKHEAP_MAXSIZE, 0);

	for (uintptr_t i = KERNEL_MAIN_KSTACK - KERNEL_STACK_SIZE; i <= KERNEL_MAIN_KSTACK; i += 0x1000)
		alloc_frame(get_page(i, 1, get_current_dir()), 1, 0);

	for (uintptr_t i = KERNEL_MAIN_STACK - KERNEL_STACK_SIZE; i <= KERNEL_MAIN_STACK; i += 0x1000)
		alloc_frame(get_page(i, 1, get_current_dir()), 1, 0);

	asm volatile("mov %0, %%esp; mov %%esp, %%ebp" : : "r"(KERNEL_MAIN_STACK));
}

#endif /* stack.h */