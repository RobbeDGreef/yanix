#include <proc/tasking.h>
#include <mm/paging.h>

void* sbrk(intptr_t incr)
{
	int kernel = 1;
	if (get_current_task()->ring == 3)
		kernel = 0;

	for (uint32_t i = get_current_task()->program_break; i < get_current_task()->program_break + incr ; i += 0x1000)
	{
		// this will alocate a frame if the frame has not already been set 
		if (alloc_frame(get_page(i, 1, get_current_task()->directory), kernel, kernel?0:1) == -2) 
		{
			return (void*) -1;
		}

	}
	get_current_task()->program_break += incr;
	return (void*) (get_current_task()->program_break - incr);
}