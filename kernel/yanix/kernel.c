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
	char **envvars = make_envvars();
	const char **args = (const char **) make_args(3, "/bin/smlrc", "/etc/hello_nostd.c", "/testout.asm");

	if (fork() == 0)
	{
		execve_user("/bin/clear", args, envvars);
	}
}
