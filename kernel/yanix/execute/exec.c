#include <fs/vfs.h>
#include <stdint.h>
#include <stdarg.h>
#include <yanix/elf.h>
#include <mm/heap.h>
#include <errno.h>
#include <proc/tasking.h>
#include <debug.h>
#include <yanix/stack.h>
#include <asm/system.h>

extern task_t *g_runningtask;

int execve_user(const char *filename, char *const argv[], char *const envp[])
{
	(void) (envp);

	/* Read the file for reading */
	struct file *file = vfs_open(filename, 0, 0);
	if (file == 0) {
		return -1;
	}
	
	/* Allocate a buffer for the file contents */
	void *buf = (void*) kmalloc(file->filesize);

	if (buf == 0) {
		/* Cleanup */
		vfs_close(file);
		
		return -1;
	}

	/* Read the file into the buffer */
	int size = vfs_read(file, buf, file->filesize);
	if (size == -1 || size == 0 || (unsigned int) size != file->filesize)
	{
		/* Cleanup */
		vfs_close(file);
		kfree(buf);

		return -1;
	}

	/* Interpret our elf executable and load it into the propper position */
	uint32_t ret = load_elf_into_mem(buf);

	if (ret == 0) {
		
		/* Cleanup */
		vfs_close(file);
		kfree(buf);
		
		return -1;
	}

	/* Count the arguments given */	
	size_t amount = 0;
	while (argv[amount] != 0) {
		amount++;
	}
	amount += 2;

	/* Now we need to switch the task to usermode, i.e. giving it brand new usermode stack */
	
	/* First disable interrupts because we are working on the stack (not sure if it's necesairy but it wont hurt) */
	cli();

	/* And allocate a new stack */
	g_runningtask->stacktop = (uint32_t) kmalloc_user_base(USER_STACK_SIZE, 1, 0) + USER_STACK_SIZE;
	g_runningtask->stack_size = USER_STACK_SIZE;


	asm volatile("mov %0, %%eax; \
				  mov %1, %%ebx; \
				  mov %%ebx, %%esp; \
				  mov %%esp, %%ebp; \
				  sti; \
				  push %%eax; \
				  call jump_userspace"
				  : : "r"(ret), "r"(g_runningtask->stacktop));
	return 0;
}

int execve(const char *filename, char const *argv[], char *const envp[])
{
	(void) (envp);

	/* Read the file for reading */
	struct file *file = vfs_open(filename, 0, 0);
	if (file == 0) {
		return -1;
	}
	
	/* Allocate a buffer for the file contents */
	void *buf = (void*) kmalloc(file->filesize);

	/* Read the file into the buffer */
	int size = vfs_read(file, buf, file->filesize);
	if (size == -1){
		return -1;
	}
	
	/* Interpret our elf executable and load it into the propper position */
	uint32_t ret = load_elf_into_mem(buf);

	if (ret == 0) {
		return -1;
	}

	/* Count the arguments given */	
	size_t amount = 0;
	while (argv[amount] != 0) {
		amount++;
	}
	amount += 2;

	/* Execute the program */
	int (*program)(int amount, char const *argv[]) = (int (*) (int amount, char const *argv[])) ret;
	program(amount, argv);
	kill_proc(g_runningtask);	// kill ourselves because we shouldn't ever return
	return 0; 
}
