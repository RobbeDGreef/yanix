#include <kernel.h>
#include <yanix/exec.h>

/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel boot up procedure completed\n");

	execve_user("/bin/testfile4",0,0);
}
