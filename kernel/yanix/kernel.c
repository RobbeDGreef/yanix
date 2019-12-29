#include <cpu/cpu.h>						/* Architechture dependand header (init) */
#include <drivers/ps2/keyboard.h>			// ps2 keyboard 				(init)
#include <drivers/ps2/mouse.h>				// ps2 mouse 					(init)
#include <fs/vfs.h>							// virtual file system			(init)
#include <drivers/pci.h>					// PCI device driver 			(init)
#include <fs/filedescriptor.h>		 		// file descriptor system 		(init)
#include <drivers/video/video.h>			// general video system 		(init)
#include <drivers/video/vesa.h>				// vesa driver 					(init)
#include <drivers/video/vga.h> 				/* VGA driver 					(init)	*/
#include <drivers/serial.h> 				/* Serial connection driver 	(init 	*/
#include <core/timer.h>						// cpu timer 					(init)
#include <drivers/ramdisk.h>				// ramdisk 						(init)
#include <proc/tasking.h>					// tasking 						(init)
#include <mm/paging.h>						// paging 						(init)
#include <mm/heap.h>						// kernel heap 					(init)
#include <proc/syscall.h>					// the system calls 			(init)
#include <yanix/stack.h>					// stack functions 				(kernel init)
#include <yanix/tty_dev.h>					// TTY functionality 			(kernel init)

#include <fs/fs.h>							// the file system api

#include <yanix/kfunctions.h>	// kernel functions

#include <libk/function.h>
#include <libk/string.h>

#include <stdint.h>
#include <kernel.h>

#define RAMDISK_LOCATION 	0x10000
#define RAMDISK_SIZE 		0x60000

extern task_t *g_starttask;
extern task_t *g_runningtask;

extern void kernel_handle_debug(void);

/**
 * @brief      Start of the two stage boot sequence
 *
 * @param[in]  stack     The stack
 * @param[in]  code_gdt  The code gdt
 * @param[in]  data_gdt  The data gdt
 */
void bootsequence(uint32_t stack)
{
	/* initialize the kernel stack */
	init_stack(stack);

	/* Initialise the serial connection early on to print to the serial before tty is available */
	init_serial();

	/* Initialize the video driver and clearing the screen */
	video_clear_screen();
	init_vesa((void*) 0xfd000000, 1024, 768, 3);
	init_video(VIDEO_MODE_VESA);
	//init_vga(0xb8000, 80, 25, 2);
	//init_video(VIDEO_MODE_TERM);
	

	arch_init();

	/* Enable the kernel timer */
	init_timer();

	/* Enable paging */
	init_paging();

}

/**
 * @brief      The second part of the boot sequence
 */
void bootsequence_after_paging()
{
	int ret = 0;

	/* Initialize the kernel heap allocation systems */
	init_kheap();
	message("Kernel heap initialized", 1);


	/* Initialize the user heap allocation systems */
	init_uheap();
	message("User heap initialized", 1);


	/* Initialize the tasking module */
	init_tasking();
	g_starttask = g_runningtask;	/* Because of stack stuff this needs to happen here */
	message("Tasking initialized", 1);
	
	/* Installing ramdisk */
	//init_ramdisk(RAMDISK_LOCATION, RAMDISK_SIZE);
	//message("Ramdisk initialized", 1);

	/* Scan PCI bus */
	init_pci();
	message("PCI driver initialized", 1);
	
	/* initialize all the pci devices with a driver */
	init_pci_devices();
	message("PCI devices initialized", 1);

	/* Initialize the file descriptors and terminal system */
	init_filedescriptors();
	message("File descriptors initialized", 1);
	init_tty_devices();
	message("TTY devices initialized", 1);
	init_tty_filedescriptors();
	message("TTY initialized", 1);

	/* Initialize the virtual file system switch */
	ret = init_vfs();
	//ret = 1;
	message("VFS initialized", !ret);

	/* Initialize the mouse and keyboard drivers */
	init_mouse();
	message("Mouse initialized", 1);

	init_keyboard();
	message("Keyboard initialized", 1);
	
	// initialize the system calls
	init_syscalls();
	message("Syscalls initialized", 1);
	
	// sleep to read the messages
	sleep(500);

	// cleanup
	//clear_screenk();
}


#include <errno.h>
#include <debug.h>
#include <yanix/exec.h>
#include <drivers/ata.h>
/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk(KERN_INFO "kernel booted\n");

	execve_user("/bin/stdin_yanix",0,0);
}

/**
 * @brief      Just a halting cpu loop forever 
 */
void enter_foreverloop()
{
	// execute halt instruction forever
	
	printk("\nEnd of main kernel loop, exiting...\n");
	while (1) {
		asm volatile ("hlt");
	}
}

/**
 * @brief      The enter function of the kernel
 *
 * @param[in]  stack     The stack
 * @param[in]  code_gdt  The code gdt
 * @param[in]  data_gdt  The data gdt
 */
void _enter(uint32_t stack)
{
	// enter function for kernel
	// starts boot sequence
	bootsequence(stack);
	
	// maps the stack to the wanted location
	init_paging_stack();
	message("Successfully remapped kernel stack to desired location", 1);

	// paging part of boot sequence
	bootsequence_after_paging();
	
	// jump to main kernel code
	kernel_main();

	// after main kernel code enter a forever loop of halt instructions
	enter_foreverloop();
}
