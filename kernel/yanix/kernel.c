#include <kernel.h>
#include <yanix/exec.h>
#include <errno.h>

/* Debugging */

#include <yanix/env.h>
#include <fs/vfs_node.h>

extern vfs_node_t *g_vfs_root;

/* Debugging */

#include <debug.h>
#include <mm/paging.h>

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel boot up procedure completed\n");

	char **envvars = make_envvars();
	const char **args = (const char **) make_args(2, "/bin/figlet", "Hello world");

	if (fork() == 0)
	{
		printk("Executing exeve\n");
		int ret = execve_user("/bin/fork", args, envvars);
		if (ret)
			printk(KERN_WARNING "Main execve returned, error was thrown: %i errno: %i\n", ret, errno);
	}

}
