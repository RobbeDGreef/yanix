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

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel boot up procedure completed\n");
	//char **envvars = make_envvars();
	//const char **args = (const char **) make_args(3, "/bin/figlet", "Hello world");

	if (fork() == 0)
	{
		printk(KERN_NOTICE "Child is running %x %x\n", get_current_task(), get_current_dir());
		if (fork() == 0)
		{
			if (fork() == 0)
			{
				printk(KERN_NOTICE "Reforked child: %x %x\n", get_current_task(), get_current_dir());
				execve_user("/bin/fork", 0, 0);
			}
		}
	}
	else
	{
		printk(KERN_NOTICE "Parent running %x %x\n", get_current_task(), get_current_dir());
	}
}
