#ifndef TASKING_H_
#define TASKING_H_

#include <sys/types.h>
#include <mm/paging.h>

#include <stddef.h>
#include <stdint.h>

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
	pid_t			pid;
	uint32_t 		ring;
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

int exit_proc(int status);

#endif