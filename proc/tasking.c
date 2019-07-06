/**
 * mm/paging.c
 * 
 * Author: Robbe De Greef
 * Date:   29 may 2019
 * 
 * Version 2.2
 */

#include <sys/types.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <kernel/stack/stack.h>
#include <drivers/mouse/ps2.h>
#include <lib/string/string.h>
#include <cpu/timer.h>
#include <cpu/gdt.h>

#include <proc/tasking.h>

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stddef.h>

#include <drivers/video/videoText.h>

#define TASK_RUNNING 	0
#define TASK_ZOMBIE 	1
#define TASK_SLEEPING 	2
#define TASK_PAUSED 	3

task_t *g_starttask = 0;
task_t *g_runningtask = 0;
pid_t PIDS = 1;

// extern page directory refrences
extern page_directory_t *g_current_directory;
extern page_directory_t *g_kernel_directory;

// extern assebmly functions
extern uint32_t get_eip();
extern void task_switch(uint32_t eip, uint32_t esp, uint32_t ebp, uint32_t cr3);

/**
 * @brief      Adds a task to the task queue.
 *
 * @param      task_to_add  The task to add
 */
void add_task_to_queue(task_t *task_to_add)
{
	task_t *tmp = g_runningtask;
	while (tmp->next != 0){
		tmp = tmp->next;
	}
	tmp->next = task_to_add;
}

/**
 * @brief      Removes a task from the task queue.
 *
 * @param      task_to_remove  The task to remove
 *
 * @return     Successcode
 */
int remove_task_from_queue(task_t *task_to_remove)
{
	task_t *tmp = g_starttask;
	while (tmp->next != task_to_remove) {
		if (tmp->next == 0) {
			errno = ESRCH; // no such process
			return -1;
		}
	}
	// tmp now houses the task before the to remove task
	tmp->next = task_to_remove->next;

	// snipped task out of queue
	return 0;
	
}


/**
 * @brief      Kills a task
 *
 * @param      task  The task to kill
 */
static void kill_proc(task_t *task)
{
	remove_task_from_queue(task);
	kfree((void*) task->directory);
	kfree((void*) task);
}

/**
 * @brief      Kills dead processes
 */
void schedule_killer()
{
	// loops over the processes and kills processes that should die
	
	task_t *tmp = g_starttask;
	while (tmp != 0) {
		if (tmp->state == TASK_ZOMBIE) {
				kill_proc(tmp);
				break;
		}
		tmp = tmp->next;
	}
}


/**
 * @brief      Switches task
 *
 * @param      nexttask  The next task
 */
static void switch_task(task_t *nexttask)
{
	uint32_t eip, esp, ebp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp)); 

	eip = get_eip();

	if (eip == 0) {
		return;
	}

	g_runningtask->eip = eip;
	g_runningtask->ebp = ebp;
	g_runningtask->esp = esp;

	g_runningtask = nexttask;

	esp = g_runningtask->esp;
	ebp = g_runningtask->ebp;

	task_switch(eip, esp, ebp, g_runningtask->directory->physicalAddress);
}



/**
 * @brief      yields control of task
 */
void task_yield()
{
	#if 0
	task_t *next;
	if (g_runningtask->next != 0){
		// switch to next task in list

		tss_set_kernel_stack(g_runningtask->next->kernel_stack+KERNEL_STACK_SIZE);
		next = g_runningtask->next;

	} else if (g_runningtask != g_starttask){
		// switch to first task

		tss_set_kernel_stack(g_starttask->kernel_stack+KERNEL_STACK_SIZE);
		next = g_starttask;
	} else {
		// no task to switch to continue the current process
		return;
	}
	if ()
	switch_task(next);
	#endif

	schedule_killer();

	task_t *tmp = g_runningtask;
	while (1) {
		if (tmp->next != 0) {
			tmp = tmp->next;
		} else if (tmp != g_starttask) {
			tmp = g_starttask;
		} else {
			// no task
			return;
		}

		if (tmp->state != TASK_ZOMBIE && tmp->state != TASK_PAUSED && tmp->state != TASK_SLEEPING) {
			break;
		} else {
			if (tmp->next != 0) {
				tmp = tmp->next;
			} else if (tmp != g_starttask) {
				tmp = g_starttask;
			} else {
				// no task
				return;
			}
		}
	}
	tss_set_kernel_stack(tmp->kernel_stack+KERNEL_STACK_SIZE);
	switch_task(tmp);
}

/**
 * @brief      Creates a task.
 *
 * @param      dir   The page directory
 *
 * @return     Pointer to the task
 */
static task_t *create_task(page_directory_t *dir)
{
	// create new task struct
	task_t *new_task = (task_t*) kmalloc(sizeof(task_t));
	memset(new_task, 0, sizeof(task_t));

	// create fresh kernel stack
	new_task->kernel_stack = (uint32_t) kmalloc_base(KERNEL_STACK_SIZE, 1, 0);
	memset((void*) new_task->kernel_stack, 0, KERNEL_STACK_SIZE);

	// set pid and new page directory (addr space)
	new_task->pid = PIDS++;
	new_task->directory = dir;
	new_task->state = TASK_RUNNING;

	new_task->timeslice = 100;

	return new_task;
}

/**
 * @brief      Forks a process 
 *
 * @return     returns the process pid (0 if this process is the child)
 */
pid_t fork()
{
	// disable interrupts
	asm volatile ("cli");

	// save for later refrence
	task_t *parent_task = g_runningtask;

	// duplicate page directory
	page_directory_t *copied_dir = duplicate_current_page_directory();

	// create a new task with new address space
	task_t *new_task = create_task(copied_dir);

	// copy the stack to the new addr space
	copy_stack_to_new_addressspace(copied_dir);

	// entry point for func
	uint32_t eip = get_eip();

	if (g_runningtask == parent_task) {
		uint32_t esp, ebp;
		asm volatile("mov %%esp, %0" : "=r"(esp));
		asm volatile("mov %%ebp, %0" : "=r"(ebp));
		new_task->esp = esp;
		new_task->ebp = ebp;
		new_task->eip = eip;

		add_task_to_queue(new_task);

		asm volatile("sti");
		return new_task->pid;
	} else {
		return 0;
	}

}



/**
 * @brief      Initializes tasking
 *
 * @return     Successcode
 */
int init_tasking()
{
	// create new task struct
	g_starttask = (task_t*) kmalloc(sizeof(task_t));
	memset(g_starttask, 0, sizeof(task_t));

	g_starttask->pid = PIDS++;
	g_starttask->directory = g_current_directory;

	g_runningtask = g_starttask;

	return 0;
}


/**
 * @brief      Sends a signal to a task.
 *
 * @param      task    The task
 * @param[in]  signal  The signal
 */
static void send_task_signal(task_t *task, int signal)
{
	if (task != 0) {
		if (task->notify != 0) {
			task->notify(signal);
		}

		switch (signal) {
			case SIGKILL:
				task->state = TASK_ZOMBIE;
				break;

			case SIGILL:
				task->state = TASK_ZOMBIE;
				break;

			case SIGTERM:
				task->state = TASK_ZOMBIE;
				break;

			case SIGSEGV:
				task->state = TASK_ZOMBIE;
				break;

			case SIGSTOP:
				task->state = TASK_PAUSED;
				break;

			case SIGCONT:
				task->state = TASK_RUNNING;
				break;
		}
		
	}
}


/**
 * @brief      Sends a signal to the current task.
 *
 * @param[in]  signal  The signal
 */
void send_sig(int signal)
{
	send_task_signal(g_runningtask, signal);
}

/**
 * @brief      Sends a pid a signal.
 *
 * @param[in]  pid   The pid
 * @param[in]  sig   The signal
 *
 * @return     success
 */
int send_pid_sig(pid_t pid, int sig)
{
	task_t *tmp = g_starttask;
	while (tmp != 0) {
		if (tmp->pid == pid) {
			send_task_signal(tmp, sig);
			return 0;
		}
		tmp = tmp->next;
	}
	errno = EINVAL;
	return -1;
}

/**
 * @brief      Schedule function (simple round robin)
 * 
 */
void schedule()
{
	// every schedule call update the mouse poll driver
	mouse_poll();

	if (g_runningtask != 0){
		if (g_runningtask->sliceused < g_runningtask->timeslice){
			// run
			g_runningtask->sliceused += (1000/g_timer_frequency);
		} else {
			// switch
			g_runningtask->sliceused = 0;
			task_yield();
		}
	} else {
		// exit gracefully because no tasking initialised
	}
}

/**
 * @brief      Jump to userspace
 */
inline void jump_userspace()
{
	tss_set_kernel_stack(g_runningtask->kernel_stack + KERNEL_STACK_SIZE);
	
	g_runningtask->ring = 3;

	asm volatile("cli; \
     			  mov $0x23, %ax; \
     			  mov %ax, %ds; \
     			  mov %ax, %es; \
     			  mov %ax, %fs; \
     			  mov %ax, %gs; \
     			                \
     			  mov %esp, %eax; \
     			  pushl $0x23; \
     			  pushl %eax; \
     			  pushf; \
     			  pushl $0x1B; \
     			  push $1f; \
    			  iret; \
   				  1: ");
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

/**
 * @brief      Typical sbrk that will increase the program break
 *
 * @param[in]  incr  The increment value
 *
 * @return     On success the old program break, on failure 0
 */
caddr_t sbrk(int incr)
{
	for (uint32_t i = g_runningtask->program_break; i < g_runningtask->program_break + incr ;i += 0x1000) {
		// this will alocate a frame if the frame has not already been set 
		int kernel = 1;
		if (g_runningtask->ring == 3) {
			kernel = 0;
		}
		if (alloc_frame(get_page(i, 1, g_runningtask->directory), kernel, kernel?0:1) == -2) {
			// TODO: should also deallocate the frame 
			errno = ENOMEM;
			return (caddr_t) -1;
		}
	}
	g_runningtask->program_break += incr;
	
	return (caddr_t) (g_runningtask->program_break -= incr);
}