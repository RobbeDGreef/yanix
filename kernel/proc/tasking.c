#include <cpu/interrupts.h>
#include <eclib/vector.h>
#include <errno.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <proc/arch_tasking.h>
#include <proc/sched.h>
#include <proc/tasking.h>
#include <sys/types.h>
#include <kernel/stack.h>
#include <proc/threads.h>

#include <debug.h>
static pid_t PIDS = 1;

/* Define the vector functions for the sighandler vector */
define_vector_functions(struct sighandler, vec_sig);

sighandler_t signal(int sig, sighandler_t handler)
{
	struct sighandler hdl = {.sig = sig, .handler = handler};
	vec_sig_push(get_current_task()->sighandlers, hdl);

	return handler;
}

static sighandler_t findsighandler(vec_sig vector, int sig)
{
	int i;
	while ((i = vec_sig_iter(vector)) != -1)
	{
		if (vec_sig_get(vector, i).sig == sig)
		{
			vector->iter = 0;
			return vec_sig_get(vector, i).handler;
		}
	}

	return NULL;
}

int send_task_signal(task_t *task, int sig)
{
	sighandler_t handler;
	if (task)
	{
		switch (sig)
		{
		case SIGINT:
			handler = findsighandler(task->sighandlers, sig);
			if (handler)
			{
				handler(sig);
				break;
			}

			kill_proc(task);
			break;
		}
	}
	return 0;
}

void send_sig(int signal)
{
	send_task_signal(get_current_task(), signal);
}

int send_pid_sig(pid_t pid, int signal)
{
	return send_task_signal(find_task_by_pid(pid), signal);
}

static void free_task(task_t *task)
{
	for (;;)
		;
	vector_destroy(task->fds);
	/* @todo: */
}

void kill_proc(task_t *task)
{
	task->state = TASK_ZOMBIE;

	/* this will return -1 if the task isn't blocked */
	if (task->parent)
	{
		task_t *parent = find_task_by_pid(task->parent);
		parent->childamount--;
		task_resume(task->parent);
	}

	/* @todo: should set task to zombie and then kill
	 *  it whenever killer is lauched, after that free all the structures */
	remove_from_ready_list(task);
	if (get_current_task() == task)
	{
		task_t *next = get_next_task();
		switch_task(next);
	}
}

/* @todo: Remove this function */
static inline void setup_stack_heap(struct heap *ptr, int um)
{
	create_heap(ptr, USER_STACKHEAP_START, USER_STACKHEAP_MAXSIZE, um);
}

static task_t *create_task(task_t *new_task, int kernel_task,
                           page_directory_t *dir)
{
	//new_task->kernel_stack = DISIRED_KERNEL_STACK_LOC;

	/* create fresh normal stack */
	if (!kernel_task)
	{
		//new_task->stacktop   = DISIRED_USER_STACK_LOC;
		//new_task->stack_size = USER_STACK_SIZE;
		new_task->ring       = 3;
	}
	else
	{
		//new_task->stacktop   = DISIRED_STACK_LOCATION;
		//new_task->stack_size = STACK_SIZE;
		new_task->ring       = 0;
	}

	task_t *curtask = get_current_task();

	/* set pid and new page directory (addr space) */
	new_task->pid       = PIDS++;
	new_task->parent    = getpid();
	new_task->uid       = curtask->uid;
	new_task->gid       = curtask->gid;
	new_task->name      = curtask->name;
	new_task->priority  = curtask->priority;
	new_task->directory = dir;
	// new_task->state = TASK_RUNNING;
	new_task->fds         = vector_copy(curtask->fds);
	new_task->tty         = curtask->tty;
	new_task->state       = TASK_RUNNING;
	new_task->sighandlers = vec_sig_copy(curtask->sighandlers);
	new_task->threads     = vec_thrds_create();
	setup_stack_heap(&new_task->stack_heap, !kernel_task);
	threads_init_forked(new_task, curtask);

	//init_thread_setup(new_task, stack_alloc(kernel_task, &new_task->stack_heap));

	/* @todo: timeslices should be set in config file */
	new_task->timeslice = 100;
	curtask->childamount++;
	new_task->cwd = strdup(curtask->cwd);

	return new_task;
}

pid_t fork()
{
	debug_printk("Forking\n");
	disable_interrupts();

	/* duplicate page directory */
	task_t *parent_task = (task_t *) get_current_task();

	task_t *new_task = (task_t *) kmalloc(sizeof(task_t));
	memcpy(new_task, parent_task, sizeof(task_t));

	create_task(new_task, parent_task->ring ? 0 : 1, 0);
	add_task_to_queue(new_task);

	printk("Current stacktop %x\n", get_current_stacktop());
	printk("New stacktop: %x\n", vec_thrds_get(new_task->threads, 0)->stack_top);
	arch_spawn_task(&(get_main_thrd(new_task)->stack), &new_task->directory);

	if (parent_task == get_current_task())
	{
		enable_interrupts();
		return new_task->pid;
	}
	else
	{
		// printk("Child\n");
		return 0;
	}
}

int init_tasking()
{
	/* @todo: this is ugly find a better solution */

	/* create new task struct */
	task_t *mainloop = kmalloc(sizeof(task_t));
	memset((task_t *) mainloop, 0, sizeof(task_t));

	mainloop->pid       = PIDS++;
	mainloop->directory = get_current_dir();
	// mainloop->state 		= TASK_RUNNING;
	mainloop->timeslice = 100;

	mainloop->uid      = 0;
	mainloop->euid     = 0;
	mainloop->gid      = 0;
	mainloop->egid     = 0;
	mainloop->name     = "Main kernel loop";
	mainloop->cwd      = "/root";
	mainloop->priority = 0;
	mainloop->fds      = vector_create();

	mainloop->sighandlers = vec_sig_create();
	mainloop->threads = vec_thrds_create();
	setup_stack_heap(&mainloop->stack_heap, 0); /* @todo: no magic values */

	init_thread_setup(mainloop, KERNEL_MAIN_STACK);

	init_scheduler(mainloop);
	return 0;
}

int task_wait(int *status)
{
	if (get_current_task()->childamount)
	{
		*status = 0;
		task_block(getpid());
		return 0;
	}
	else
	{
		errno = ECHILD;
		return -1;
	}
}

unsigned long get_proccount()
{
	return task_count_all();
}

uid_t task_euid()
{
	return get_current_task()->euid;
}

gid_t task_egid()
{
	return get_current_task()->egid;
}

uintptr_t get_current_stacktop()
{
	struct thread *t = vec_thrds_get(get_current_task()->threads, 0);
	return t->stack_top;
}

struct thread *get_main_thrd(task_t *task)
{
	return vec_thrds_get(task->threads, 0);
}

struct thread *get_current_thrd()
{
	/* @todo: */
	return vec_thrds_get(get_current_task()->threads, 0);
}