#include <asm/system.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <proc/tasking.h>
#include <yanix/elf.h>
#include <yanix/env.h>
#include <yanix/stack.h>

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>

#include <kernel.h>

#include <libk/string.h>

#include <debug.h>
#include <mm/linkedlist.h>

static int _execve(int jmpuser, const char *filename, const char **argv,
                   char const **envp)
{
	/* Read the file for reading */
	struct file *file = vfs_open(filename, 0, 0);
	if (file == 0)
		return -1;

	if (!argv)
		goto noargs_debug;
	/**
	 * We have to assemble the argument list now because we will
	 * most likely overwrite the list and more importantly the string
	 * while loading the elf file into memory
	 */
	size_t amount = 0;
	while (argv[amount] != 0)
		amount++;

	argv = (const char **) combine_args_env((char **) argv, (char **) envp);
	if (!argv)
	{
		vfs_close(file);
		return -1;
	}

noargs_debug:;
	/* Interpret our elf executable and load it into the propper position */
	offset_t ret = load_elf_into_mem(file);

	if (ret == 0)
	{
		vfs_close(file);
		return -1;
	}

	/* Cleanup */
	vfs_close(file);

	get_current_task()->name = (char *) filename;

	debug_printk(KERN_INFO "Executing execve\n");

	if (!jmpuser)
	{
		/* Execute the program */
		asm volatile("pushl %2; \
					   pushl %1; \
					   jmp %0;"
		             :
		             : "r"(ret), "r"(amount), "r"(argv));

		return 0;
	}
	else
	{
		/* Now we need to switch the task to usermode, i.e. giving it brand new
		 * usermode stack */

		/* First disable interrupts because we are working on the stack (not
		 * sure if it's necessary but it wont hurt) */
		asm volatile("cli;");

		/* Sets up a user stack */
		set_user_stack();

		asm volatile("movl %0, %%eax; \
					  movl %1, %%ebx; \
					  movl %2, %%ecx; \
					  movl %3, %%edx; \
					  movl %%ebx, %%esp; \
					  movl %%esp, %%ebp; \
					  pushl %%edx; \
					  pushl %%ecx; \
					  pushl %%eax; \
					  call jump_userspace;"
		             :
		             : "r"(ret), "r"(get_current_task()->stacktop), "c"(amount),
		               "d"((reg_t) argv));
		return 0;
	}
}

int execve_user(const char *filename, const char **argv, char const **envp)
{
	return _execve(1, filename, argv, envp);
}

int execve(const char *filename, const char **argv, char const **envp)
{
	return _execve(0, filename, argv, envp);
}
