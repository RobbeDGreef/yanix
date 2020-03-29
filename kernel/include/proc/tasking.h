#ifndef TASKING_H_
#define TASKING_H_

#include <sys/types.h>
#include <mm/paging.h>
#include <yanix/ds/fd_vector.h>

#include <stddef.h>
#include <stdint.h>

/* Function pointer template for a notify function to notify the task of changes in it's signal */
typedef void (*notify_fpointer) (int signal);

typedef struct task_control_block_s
{
	/* Registers used for switching */
	uint32_t 			eip;
	uint32_t 			esp;
	uint32_t 			ebp;
	uint32_t			eflags;
	page_directory_t 	*directory;			/* Program page directory */

	/* Program information */
	uint32_t			program_start; 		/* The start of the program */
	uint32_t			program_break;		/* Program break */
	uint32_t	 		stacktop;			/* Program's stack top */
	uint32_t			stack_size;			/* Program's stack size */

	uint32_t 			kernel_stack;		/* Kernel stack (same as stacktop if this is a kernel task) */

	/* Task information */
	char 				*name;
	pid_t 				pid;
	uid_t				uid;
	gid_t				gid;
	int 				ring;
	vector_t			*fds;				/* The opened file descriptors */

	/* Signaling */
	int 				lastsignal;			/* The last signal sent to this task */
	unsigned int 		tty;				/* What tty to wirte output to */
	notify_fpointer 	notify;				/* Notify pointer, programs can register this pointer to handle signals better */

	/* Scheduler information */
	int 				state;				/* Current state of the task */
	unsigned long 		timeslice;			/* The size of the timeslice of this task */
	unsigned long 		timeused;			/* The total amount of cpu time this task has used */
	unsigned long 		sliceused;			/* The current amount of cpu time the task is using */
	int 				priority;			/* The priority of the task */
	int 				spawned;			/* Whether the task was already spawned or not */

	/* Linked list next identifier */
	struct task_control_block_s *next;

} __attribute__((packed)) task_t;


task_t *get_current_task();

/**
 * @brief      Switches tasks
 *
 * @param      next  The next task
 */
void switch_task(task_t *next);

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
void jump_userspace(reg_t eip, reg_t argc, reg_t argv);

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
void *sbrk(intptr_t incr);


/**
 * @brief      Kills a task
 *
 * @param      task  The task to kill
 */
void kill_proc(task_t *task);

#endif // TASKING_H_