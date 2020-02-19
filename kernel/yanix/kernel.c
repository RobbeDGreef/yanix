#include <kernel.h>
#include <yanix/exec.h>

/* Debugging */

#include <fs/vfs_node.h>

extern vfs_node_t *g_vfs_root;

/* Debugging */

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel boot up procedure completed\n");
	execve_user("/bin/testfile4",0,0);
}
