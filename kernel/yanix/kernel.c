#include <kernel.h>
#include <yanix/exec.h>
#include <errno.h>

/* Debugging */

#include <yanix/env.h>
#include <fs/vfs_node.h>

extern vfs_node_t *g_vfs_root;

/* Debugging */

#include <mm/paging.h>
#include <debug.h>
#include <mm/heap.h>
#include <yanix/user.h>
#include <libk/stdio.h>
#include <unistd.h>

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel boot up procedure completed\n");

	if (fork() == 0)
	{
		while (1)
		{
			login();

			int status;
			if (fork() == 0)
			{
				char **envvars = make_envvars();
				char **args = make_args(1, get_current_user()->shell);
				execve_user(args[0], (const char**) args, (const char**) envvars);	
				printk("Error trying to execute your shell (%s)\n", args[0]);
			}
			else
				task_wait(&status);
		}	
	}
}
