#ifndef TASKING_H_
#define TASKING_H_

#include <mm/paging.h>
#include <mm/heap.h>
#include <signal.h>
#include <sys/types.h>
#include <kernel/ds/fd_vector.h>

#include <stddef.h>
#include <stdint.h>

#include <eclib/vector.h>

define_vector_type(vec_sig);
define_vector_type(vec_thrds);
define_vector_functionprototypes(struct thread *, vec_thrds);

typedef struct task_s
{
	page_directory_t *directory; /* Program page directory */

	/* Program information */
	uintptr_t program_start; /* The start of the program */
	uintptr_t program_break; /* Program break */

	/* Task information */
	char *    name;
	char *    cwd;
	pid_t     pid;
	pid_t     parent;
	pid_t     childamount;
	uid_t     uid;
	uid_t     euid;
	mode_t    umask;
	gid_t     gid;
	gid_t     egid;
	int       ring;
	vector_t *fds; /* The opened file descriptors */

	/* Signaling */
	int          lastsignal; /* The last signal sent to this task */
	unsigned int tty;        /* What tty to wirte output to */
	vec_sig      sighandlers;

	/* Scheduler information */
	int           state;     /* Current state of the task */
	unsigned long timeslice; /* The size of the timeslice of this task */
	unsigned long timeused;  /* The total amount of cpu time
	                          * this task has used */
	unsigned long sliceused; /* The current amount of cpu time
	                          * the task is using */
	int priority;            /* The priority of the task */

	vec_thrds threads;	/*  The threads the task has (at least 1) */

	/* Used for stack management */
	struct heap stack_heap;

	/* Linked list next identifier */
	struct task_s *next;

} __attribute__((packed)) task_t;

#define TASK_RUNNING 1
#define TASK_ZOMBIE  2
#define TASK_SLEEP   3
#define TASK_BLOCKED 4

task_t *get_current_task();

typedef void (*sighandler_t)(int);
struct sighandler
{
	int          sig;
	sighandler_t handler;
};

sighandler_t signal(int sig, sighandler_t handler);

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
void jump_userspace(reg_t eip, reg_t stackstop, reg_t argc, reg_t argv);

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

int           task_wait(int *status);
unsigned long get_proccount();
uid_t         task_euid();
gid_t         task_egid();
uintptr_t get_current_stacktop();
struct thread *get_main_thrd(task_t *task);
struct thread *get_current_thrd();

#endif // TASKING_H_