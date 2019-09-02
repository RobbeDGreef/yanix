#include <drivers/keyboard/ps2.h>			// ps2 keyboard 				(init)
#include <drivers/mouse/ps2.h>				// ps2 mouse (for gui)			(init)
#include <drivers/vfs/vfs.h>				// virtual file system			(init)
#include <drivers/pci/pci.h>				// PCI device driver 			(init)
#include <fs/filedescriptor.h>		 		// file descriptor system 		(init)
#include <drivers/video/video.h>			// general video system 		(init)
#include <drivers/video/vesa.h>				// vesa driver 					(init)
#include <cpu/timer.h>						// cpu timer 					(init)
#include <cpu/isr.h>						// interrupt service routines 	(init)
#include <cpu/gdt.h>						// global descriptor table		(init)
#include <drivers/ramdisk/ramdisk.h>		// ramdisk 						(init)
#include <proc/tasking.h>					// tasking 						(init)
#include <mm/paging.h>						// paging 						(init)
#include <mm/heap.h>						// kernel heap 					(init)
#include <proc/syscall.h>					// the system calls 			(init)
#include <kernel/stack/stack.h>				// stack functions 				(kernel init)
#include <kernel/tty/tty_dev.h>				// TTY functionality 			(kernel init)

#include <fs/fs.h>							// the file system api

#include <config/kconfig.h>					// kernel configuration file
#include <kernel/functions/kfunctions.h>	// kernel functions

#include <libk/function.h>
#include <libk/string/string.h>

#include <stdint.h>
#include <kernel.h>


#define RAMDISK_LOCATION 	0x10000
#define RAMDISK_SIZE 		64*0x200

extern task_t *g_starttask;
extern task_t *g_runningtask;

/**
 * @brief      Start of the two stage boot sequence
 *
 * @param[in]  stack     The stack
 * @param[in]  code_gdt  The code gdt
 * @param[in]  data_gdt  The data gdt
 */
void bootsequence(uint32_t stack, uint32_t code_gdt, uint32_t data_gdt)
{
	/* initialize the kernel stack */
	init_stack(stack);

	/* Unused variables */
	UNUSED(code_gdt); UNUSED(data_gdt);

	/* Initialize the video driver and clearing the screen */
	video_clear_screen();
	init_vesa((void*) 0xfd000000, 1024, 768, 3);
	init_video(VIDEO_MODE_VESA);
	
	/* Installing the interrupt service routines and enabling interrupts */
	init_descriptor_tables();
	isr_install();
	asm volatile ("sti");

	/* Enable the kernel timer */
	init_timer(250);

	/* Enable paging */
	init_paging();
}

/**
 * @brief      The second part of the boot sequence
 */
void bootsequence_after_paging()
{

	/* Initialize the kernel heap allocation systems */
	init_kheap();
	message("Heap initialized", 1);

	/* Initialize the tasking module */
	init_tasking();
	g_starttask = g_runningtask;	/* Because of stack stuff this needs to happen here */
	message("Tasking initialized", 1);
	
	/* Installing ramdisk */
	init_ramdisk(RAMDISK_LOCATION, RAMDISK_SIZE);
	message("Ramdisk initialized", 1);
	
	/* Initialize the virtual file system switch */
	init_vfs();
	message("VFS initialized", 1);

	/* Initialize the file descriptors and terminal system */
	init_filedescriptors();
	message("File descriptors initialized", 1);
	init_tty_devices();
	message("TTY devices initialized", 1);
	init_tty_filedescriptors();
	message("TTY initialized", 1);


	/* Initialize the mouse and keyboard drivers */
	init_mouse();
	message("Mouse initialized", 1);

	init_keyboard();
	message("Keyboard initialized", 1);
	
	// initialize the system calls
	init_syscalls();
	message("Syscalls initialized", 1);
	
	// initialize pci devices
	init_pci();
	message("PCI driver initialized", 1);

	// initialize all the pci devices with a driver
	init_pci_devices();
	message("PCI devices initialized", 1);
	
	// sleep to read the messages
	sleep(TIMEAFTERBOOT);

	// cleanup
	//clear_screenk();
}


/**
 * @brief      Kernel main loop
 */
void kernel_main()
{
	printk("kernel booted\n");
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
void _enter(uint32_t stack, uint32_t code_gdt, uint32_t data_gdt)
{
	// enter function for kernel
	// starts boot sequence
	bootsequence(stack, code_gdt, data_gdt);
	
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
