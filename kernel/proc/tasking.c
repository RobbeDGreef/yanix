#include <proc/arch_tasking.h>
#include <proc/tasking.h>
#include <proc/sched.h>
#include <sys/types.h>
#include <yanix/stack.h>
#include <cpu/interrupts.h>
#include <mm/heap.h>
#include <libk/string.h>

#include <debug.h>

static pid_t PIDS = 0;

int send_task_signal(task_t *task, int sig)
{
	if (task)
	{
		if (task->notify)
			task->notify(sig);

		/* @todo: state system / sleeping,  blocking etc */
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

void kill_proc(task_t *task)
{
	/* @todo: should set task to zombie and then kill
	 *  it whenever killer is lauched, after that free all the structures */
	int r = remove_from_ready_list(task);
	if (get_current_task() == task)
	{
		task_t *task = get_next_task();
		switch_task(task);
	}
}

static task_t *create_task(task_t *new_task, int kernel_task, page_directory_t *dir)
{
	new_task->kernel_stack = DISIRED_KERNEL_STACK_LOC;
	
	/* create fresh normal stack */
	if (!kernel_task)
	{
		new_task->stacktop = DISIRED_USER_STACK_LOC;
		new_task->stack_size = USER_STACK_SIZE;
		new_task->ring = 3;
	}
	else 
	{
		new_task->stacktop = DISIRED_STACK_LOCATION;
		new_task->stack_size = STACK_SIZE;
		new_task->ring = 0;
	}
	new_task->esp = new_task->stacktop;

	task_t *curtask = get_current_task();

	/* set pid and new page directory (addr space) */
	new_task->pid = PIDS++;
	new_task->uid = curtask->uid;
	new_task->gid = curtask->gid;
	new_task->name = curtask->name;
	new_task->priority = curtask->priority;
	new_task->directory = dir;
	//new_task->state = TASK_RUNNING;
	new_task->fds = vector_copy(curtask->fds);
	new_task->tty = curtask->tty;

	/* @todo: timeslices should be set in config file */
	new_task->timeslice = 100;

	debug_printk("New pid: %i\n", new_task->pid);

	return new_task;
}

pid_t fork()
{
	disable_interrupts();

	/* duplicate page directory */
	task_t *parent_task = (task_t*) get_current_task();

	task_t *new_task = (task_t*) kmalloc(sizeof(task_t));
	memcpy(new_task, parent_task, sizeof(task_t));
	
	create_task(new_task, parent_task->ring ? 0:1, 0);
	add_task_to_queue(new_task);
	
	//printk(KERN_DEBUG "Going to spawn task %x %x\n", new_task->esp, new_task->directory);
	arch_spawn_task(&new_task->esp, &new_task->directory);
	NOTICE_POINT();
	//printk(KERN_DEBUG "Spawned %x %x\n", new_task->esp, new_task->directory);

	if (parent_task == get_current_task())
	{
		printk("Parent\n");
		alloc_frame(get_page(new_task->kernel_stack - 0x1000, 1, new_task->directory), 1, 0); 
		enable_interrupts();
		return new_task->pid;
	}
	else
	{
		printk("Child\n");
		return 0;
	}
}

int init_tasking()
{
	/* @todo: this is ugly find a better solution */

	/* create new task struct */
	task_t *mainloop = kmalloc(sizeof(task_t));
	memset((task_t*) mainloop, 0, sizeof(task_t));

	mainloop->pid 			= PIDS++;
	mainloop->directory 	= get_current_dir();
	//mainloop->state 		= TASK_RUNNING;
	mainloop->timeslice 	= 100;
	
	mainloop->kernel_stack 	= DISIRED_KERNEL_STACK_LOC;
	mainloop->stacktop 		= DISIRED_STACK_LOCATION;
	mainloop->stack_size 	= STACK_SIZE;

	mainloop->uid = 0;
	mainloop->gid = 0;
	mainloop->name = "Main kernel loop";
	mainloop->priority = 0;
	mainloop->fds = vector_create();

	init_scheduler(mainloop);
	return 0;
}
