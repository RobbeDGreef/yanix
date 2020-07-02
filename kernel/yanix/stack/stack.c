#include <mm/paging.h>
#include <proc/tasking.h>
#include <stdint.h>
#include <kernel/stack.h>

uint32_t                 g_inital_esp = 0;
extern page_directory_t *g_current_directory;

/**
 * @brief      Saves the stack location
 *
 * @param[in]  stack_location  The stack location
 */
void init_stack(uint32_t stack_location)
{
	g_inital_esp = stack_location;
}

void set_user_stack()
{
	/**
	 * Stacks always have to be preallocated because if
	 * we don't the page fault handler has no stack to
	 * run of of and thus would double and then tripple fault the cpu
	 */
	for (unsigned int i = DISIRED_USER_STACK_LOC - USER_STACK_SIZE;
	     i < DISIRED_USER_STACK_LOC;
	     i += 0x1000)
		alloc_frame(get_page(i, 1, get_current_dir()), 0, 1);

	get_current_task()->stacktop = DISIRED_USER_STACK_LOC;
}
#include <debug.h>
/**
 * @brief      Initializes the stack with paging and bind it to the disired
 * virtual address
 */
void init_paging_stack(reg_t stacktop)
{
	// this is just to make sure the page after our stack is present for a bug
	// in copy_page
	map_physical_to_virtual((phys_addr_t *) (stacktop - INIT_STACK_SIZE),
	                        (void *) (DISIRED_STACK_LOCATION - INIT_STACK_SIZE),
	                        INIT_STACK_SIZE, 1, 0, g_current_directory);

	uint32_t esp, ebp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));

	debug_printk("stack: %x %x %x %x %x %x\n", DISIRED_STACK_LOCATION,
	             KERNEL_STACK_SIZE, stacktop, esp, stacktop - esp,
	             DISIRED_STACK_LOCATION - ((stacktop - esp)));
	asm volatile("mov %0, %%esp"
	             :
	             : "r"(DISIRED_STACK_LOCATION - (stacktop - esp)));
	asm volatile("mov %0, %%ebp"
	             :
	             : "r"(DISIRED_STACK_LOCATION - (stacktop - ebp)));

	map_mem(DISIRED_STACK_LOCATION, DISIRED_STACK_LOCATION + KERNEL_STACK_SIZE,
	        1, 0);

	for (uint i = 0; i < KERNEL_STACK_SIZE; i += 0x1000)
		alloc_frame(get_page(DISIRED_STACK_LOCATION - i, 1, get_current_dir()),
		            0, 1);
}
