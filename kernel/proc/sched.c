#include <mm/paging.h>
#include <mm/heap.h>
#include <proc/tasking.h>
#include <proc/arch_tasking.h>
#include <cpu/interrupts.h>

#include <debug.h>


struct task_list
{
	task_t *head;
	task_t *next;
};

/* Normal looping ready list */
struct task_list ready_list;

/* Sleeping task list, iterate to see which needs to be fired */
struct task_list sleep_list;

/* These tasks are currently blocked */
struct task_list blocked_list;

static unsigned int 	scheduler_locks = 0;
static volatile task_t *g_runningtask   = 0;

task_t *get_current_task()
{
	return (task_t*) g_runningtask;
}

/**
 * @brief      Gets the current PID
 *
 * @return     The current PID
 */
int getpid()
{
	return g_runningtask->pid;
}

void set_list(struct task_list *list, task_t *task)
{
	list->next = task;
	list->head = task;
}

void add_to_list(struct task_list *list, task_t *task)
{
	task_t *tmp = list->head;

	if (!tmp)
	{
		set_list(list, task);
		return;
	}

	while (tmp->next)
		tmp = tmp->next;
	
	tmp->next = task;

}

int remove_from_list(struct task_list *list, task_t *task)
{
	task_t *tmp = list->head;

	if (!tmp)
	{
		return -1;
	}

	task_t *prev = tmp;
	while (tmp)
	{
		if (tmp == task)
		{
			/* remove */
			if (prev)
				prev->next = tmp->next;
			else
				list->head = tmp->next;
		}
		prev = tmp;
		tmp = tmp->next;
	}

	return -1;
}

task_t *get_next_from_list(struct task_list *list)
{

	task_t *next = list->next;

	if (list->next)
		list->next = list->next->next;

	else
		list->next = list->head;

	return next;
}

task_t *get_next_task()
{
	/* @todo: not finished, gotta check for sleeping tasks */
	return get_next_from_list(&ready_list);
}

int remove_from_ready_list(task_t *task)
{
	return remove_from_list(&ready_list, task);
}

task_t *find_task_in_list_by_pid(struct task_list *list, pid_t pid)
{
	task_t *tmp = list->head;

	while (tmp)
	{
		if (tmp->pid == pid)
			return tmp;

		tmp = tmp->next;
	}

	return 0;
}

task_t *find_task_by_pid(pid_t pid)
{
	task_t *ret = 0;
	ret = find_task_in_list_by_pid(&ready_list, pid);
	if (ret)
		goto end;

	ret = find_task_in_list_by_pid(&sleep_list, pid);
	if (ret)
		goto end;

	ret = find_task_in_list_by_pid(&blocked_list, pid);
	if (ret)
		goto end;

end:;
	return ret;
}

void add_task_to_queue(task_t *new_task)
{
	add_to_list(&ready_list, new_task);
}

void switch_task(task_t *next)
{
	task_t *previous = (task_t*) g_runningtask;
	
	if (previous == next)
		return;

	/* I am here to handle the stack that blows the fuck up
	 * oh yeah also i gotta fix that. */
	if (!next)
		return;

	g_runningtask = next;
	g_runningtask->sliceused = 0;
	/* @todo: maybe i need to set the task state here */
	
	set_current_dir(g_runningtask->directory);
	debug_printk("Arch switch to %x\n", g_runningtask);
	arch_task_switch((task_t*) g_runningtask, previous);
}

void lock_scheduler()
{
	disable_interrupts();
	scheduler_locks++;
}

void unlock_scheduler()
{
	scheduler_locks--;
	if (scheduler_locks == 0)
		enable_interrupts();
}

void schedule()
{
	lock_scheduler();

	task_t *next = get_next_task();
	
	if (next)
	{
		switch_task(next);
	}

	unlock_scheduler();	
}

void jump_userspace(reg_t eip, reg_t argc, reg_t argv)
{
	g_runningtask->ring = 3;
	arch_jump_userspace(eip, argc, argv);
}

int init_scheduler(task_t *mainloop)
{
	set_list(&ready_list, mainloop);
	set_list(&sleep_list, 0);
	set_list(&blocked_list, 0);

	g_runningtask = mainloop;

	return 0;
}