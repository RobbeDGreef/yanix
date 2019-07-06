#ifndef TASKING_H_
#define TASKING_H_

#include <sys/types.h>
#include <mm/paging.h>

#include <stddef.h>
#include <stdint.h>

typedef void (*notify_fptr) (int signal);

/**
 * @brief      a task control block 
 */
struct task_control_block_s {
	// registers
	uint32_t 			eip;
	uint32_t 			esp;
	uint32_t 			ebp;
	uint32_t			eflags;
	page_directory_t 	*directory;

	// others
	offset_t 		program_break;
	pid_t			pid;
	int 			ring;
	int 			lastsignal;
	int 			state;
	notify_fptr 	notify;
	uintptr_t 		*tty;

	uint32_t 		timeused;
	uint32_t		sliceused;
	uint32_t		timeslice;
	char 			*name;
	uint32_t 		kernel_stack;

	// next
	struct task_control_block_s *next;
}__attribute__((packed));

typedef struct task_control_block_s task_t;

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
pid_t fork();

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

#endif