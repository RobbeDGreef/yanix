#include <errno.h>
#include <kernel.h>
#include <yanix/exec.h>

/* Debugging */

#include <fs/vfs_node.h>
#include <yanix/env.h>

extern vfs_node_t *g_vfs_root;

/* Debugging */

#include <debug.h>
#include <libk/stdio.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <unistd.h>
#include <yanix/user.h>

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	if (fork() == 0)
	{
		while (1)
		{
			login();
			int status;
			if (fork() == 0)
			{
				char **envvars = make_envvars();
				char **args    = make_args(1, get_current_user()->shell);
				execve_user(args[0], (const char **) args,
				            (const char **) envvars);
				printk("Error trying to execute your shell (%s)\n", args[0]);
			}
			else
				task_wait(&status);
		}
	}
}
