#ifndef TASKING_H_
#define TASKING_H_

#include <sys/types.h>
#include <mm/paging.h>

#include <stddef.h>
#include <stdint.h>

/* Function pointer template for a notify function to notify the task of changes in it's signal */
typedef void (*notify_fpointer) (int signal);

/**
 * @brief 		A task control structure. 
 */
typedef struct task_control_block_s
{
	/* Registers used for switching */
	uint32_t 			eip;
	uint32_t 			esp;
	uint32_t 			ebp;
	uint32_t			eflags;
	page_directory_t 	*directory;

	/* Program information */
	uint32_t 			program_break;
	uint32_t	 		stacktop;
	uint32_t			stack_size;

	/* Kernel stack (same as stacktop if this is a kernel task) */
	uint32_t 			kernel_stack;

	/* Task information */
	char 				*name;
	pid_t 				pid;
	int 				ring;
	int 				lastsignal;
	int 				state;
	unsigned int 		tty;
	notify_fpointer 	notify;

	/* Scheduler information */
	unsigned long 		timeslice;
	unsigned long 		timeused;
	unsigned long 		sliceused;
	int 				priority;
	int 				spawned;

	/* Linked list next identifier */
	struct task_control_block_s *next;
} __attribute__((packed))task_t;

/**
 * @brief      yields control of task
 */
void task_yield();

/**
 * @brief      Schedule function (simple round robin)
 * 
 */
void schedule();

/**
 * @brief      Initializes tasking
 *
 * @return     Successcode
 */
int init_tasking();

/**
 * @brief      Forks a process 
 *
 * @return     returns the process pid (0 if this process is the child)
 */
pid_t fork(void);

/**
 * @brief      Jump to userspace
 */
void jump_userspace();

/**
 * @brief      Sends a signal to the current task.
 *
 * @param[in]  signal  The signal
 */
void send_sig(int signal);

/**
 * @brief      Gets the current PID
 *
 * @return     The current PID
 */
int getpid();

/**
 * @brief      Sends a pid a signal.
 *
 * @param[in]  pid   The pid
 * @param[in]  sig   The signal
 *
 * @return     success
 */
int send_pid_sig(pid_t pid, int sig);

/**
 * @brief      Typical sbrk that will increase the program break
 *
 * @param[in]  incr  The increment value
 *
 * @return     On success the old program break, on failure 0
 */
void *sbrk(int incr);


/**
 * @brief      Kills a task
 *
 * @param      task  The task to kill
 */
void kill_proc(task_t *task);

#endif