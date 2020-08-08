#include <mm/paging.h>
#include <proc/tasking.h>
#include <stdint.h>
#include <kernel/stack.h>
#include <mm/heap.h>

uintptr_t g_inital_esp = 0;
struct heap *g_kernel_stackheap = NULL;

/**
 * @brief      Saves the stack location
 *
 * @param[in]  stack_location  The stack location
 */
void init_stack(uintptr_t stack_location)
{
	g_inital_esp = stack_location;
}

void set_user_stack()
{
	//get_main_thrd(get_current_task())
}

#if 0
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

#endif

uintptr_t stack_alloc(int kernel, struct heap *sheap)
{
	size_t sz;
	struct heap *heap;

	if (kernel)
	{
		sz = KERNEL_STACK_SIZE;
		heap = g_kernel_stackheap;
	}
	else
	{
		/* User stack */
		sz = USER_STACK_SIZE;
		heap = &get_current_task()->stack_heap;
	}

	if (sheap)
		heap = sheap;

	return (uintptr_t) kmalloc_gen_base(heap, sz, 1, NULL) + sz;
}

void stack_free(int kernel, uintptr_t top)
{
	if (kernel)
		kfree_gen(g_kernel_stackheap, (void*) (top - KERNEL_STACK_SIZE));
	else
		kfree_gen(&get_current_task()->stack_heap, (void*) (top - USER_STACK_SIZE));
}

inline void __attribute__((always_inline)) init_main_stack()
{
	g_kernel_stackheap = create_heap(KERNEL_STACKHEAP_START, KERNEL_STACKHEAP_MAXSIZE, 0);

	asm volatile("mov %0, %%esp; mov %0, %%ebp" : : "r"(KERNEL_MAIN_STACK));
}
