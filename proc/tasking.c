#include <sys/types.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <kernel/stack/stack.h>
#include <drivers/mouse/ps2.h>
#include <lib/string/string.h>
#include <cpu/timer.h>
#include <cpu/gdt.h>

#include "tasking.h"

#include <stdint.h>
#include <stddef.h>

#include <drivers/video/videoText.h>

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
	if (g_runningtask->next != 0){
		// switch to next task in list
		//for(;;);
		//print_hex_dump(g_runningtask->next->regs.esp, 0x100);

		tss_set_kernel_stack(g_runningtask->next->kernel_stack+KERNEL_STACK_SIZE);
		switch_task(g_runningtask->next);
	} else if (g_runningtask != g_starttask){
		// switch to first task

		tss_set_kernel_stack(g_starttask->kernel_stack+KERNEL_STACK_SIZE);
		switch_task(g_starttask);
	} else {
		// no task to switch to continue the current process
		return;
	}
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