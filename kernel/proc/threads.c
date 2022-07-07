#include <proc/tasking.h>
#include <proc/threads.h>
#include <kernel/stack.h>
#include <libk/string.h>

define_vector_functions(struct thread *, vec_thrds);

int init_thread_setup(task_t *parent, uintptr_t top)
{
	struct thread *thrd = kmalloc(sizeof(struct thread));

	thrd->stack_top = top;
	thrd->stack = thrd->stack_top;

	thrd->kernel_stack_top = KERNEL_MAIN_KSTACK;
	thrd->kernel_stack = thrd->kernel_stack_top;

	thrd->kernel_stack_size = thrd->stack_size = KERNEL_STACK_SIZE;

	vec_thrds_push(parent->threads, thrd);

	return 0;
}

int threads_init_forked(task_t *new, task_t *old)
{
	struct thread *thrd = kmalloc(sizeof(struct thread));
	memcpy(thrd, vec_thrds_get(old->threads, 0), sizeof(struct thread));
	vec_thrds_push(new->threads, thrd);
	return 0;
}
