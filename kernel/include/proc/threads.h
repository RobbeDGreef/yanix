#ifndef _PROC_THREADS_H
#define _PROC_THREADS_H

#include <sys/types.h>

struct thread
{
	//int id;

	//task_t *owner;
	
	uintptr_t stack;		/* The stack ptr used for context switching */
	uintptr_t stack_top;
	uintptr_t stack_size;
	
	uintptr_t kernel_stack;
	uintptr_t kernel_stack_top;
	uintptr_t kernel_stack_size;

	uintptr_t entry;
};

int init_thread_setup(task_t *parent, uintptr_t stack);
int threads_init_forked(task_t *new, task_t *old);

#endif /* _PROC_THREADS_H */