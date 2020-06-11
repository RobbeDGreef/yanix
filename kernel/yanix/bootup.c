#include <core/timer.h>     /* cpu timer 					(init) */
#include <cpu/cpu.h>        /* Architechture dependand header (init) */
#include <cpu/interrupts.h> /* interrupt enable/disable 	(kernel init) */
#include <drivers/keyboard.h>
#include <drivers/pci.h>          /* PCI device driver 			(init) */
#include <drivers/ps2/keyboard.h> /* ps2 keyboard 				(init) */
#include <drivers/ps2/mouse.h>    /* ps2 mouse 					(init) */
#include <drivers/ramdisk.h>      /* ramdisk 						(init) */
#include <drivers/serial.h>       /* Serial connection driver 	(init) */
#include <drivers/video/vesa.h>   /* vesa driver 					(init) */
#include <drivers/video/video.h>  /* general video system 		(init) */
#include <fs/filedescriptor.h>    /* file descriptor system 		(init) */
#include <fs/vfs.h>               /* virtual file system			(init) */
#include <mm/heap.h>              /* kernel heap 					(init) */
#include <mm/paging.h>            /* paging 						(init) */
#include <proc/syscall.h>         /* the system calls 			(init) */
#include <proc/tasking.h>         /* tasking 						(init) */
#include <kernel/stack.h>         /* stack functions 				(kernel init) */
#include <kernel/system.h>        /* System info initialisation (init) */
#include <kernel/tty_dev.h>       /* TTY functionality 			(kernel init) */
#include <kernel/user.h>          /* User system 					(init) */

#include <kernel.h>

#include <debug.h> /* Obviously for debugging purposes */

/* Defined in kernel.c */
void kernel_main();

/**
 * @brief      Start of the two stage boot sequence
 *
 * @param[in]  stack     The stack
 */
void bootsequence(uint32_t stack)
{
	/* initialize the kernel stack */
	init_stack(stack);

	/* Initialise the serial connection early on to print to the serial before
	 * tty is available */
	init_serial();

	/* Initialize the video driver and clearing the screen */
	video_clear_screen();
	init_vesa((void *) 0xfd000000, 1024, 768, 4);
	init_video(VIDEO_MODE_VESA);

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

	ret = init_kheap();
	message("Kernel heap initialized", !ret);

	ret = init_uheap();
	message("User heap initialized", !ret);

	ret = init_tasking();
	message("Tasking initialized", !ret);

	/* Scan PCI bus */
	ret = init_pci();
	message("PCI driver initialized", !ret);

	/* initialize all the pci devices with a driver */
	ret = init_pci_devices();
	message("PCI devices initialized", !ret);

	/* Initialize the file descriptors and terminal system */
	ret = init_filedescriptors();
	message("File descriptors initialized", !ret);
	ret = init_tty_devices();
	message("TTY devices initialized", !ret);
	// ret = init_tty_filedescriptors();
	// message("TTY initialized", !ret);

	ret = init_vfs();
	message("VFS initialized", !ret);

	ret = init_char_specials();
	message("Creating special character files in fs", !ret);

	ret = init_mouse();
	message("Mouse initialized", !ret);

	ret = init_ps2_keyboard();
	message("Keyboard initialized", !ret);

	ret = init_keyboard();
	message("keyboard init", !ret);

	enable_interrupts();
	message("Enabled system interrupts", 0);

	ret = init_syscalls();
	message("Syscalls initialized", !ret);

	ret = init_user();
	message("User initialized", !ret);

	ret = init_sysinfo();
	message("System info initialized", !ret);

	/* sleep to read the messages */
	sleep(1000);

	/* cleanup */
	clear_screenk();
}

void enter_foreverloop()
{
	/* Execute halt instruction forever */
	while (1)
	{
		asm volatile("hlt");
	}
}

/**
 * @brief      The kernel enter function
 *
 * @param[in]  stack     The current stack pointer
 */
void _enter(uint32_t stack)
{
	/* Make sure interrupts are disabled */
	disable_interrupts();
	bootsequence(stack);

	/* maps the stack to the wanted location */
	init_paging_stack();

	message("Successfully remapped kernel stack to desired location", 1);
	bootsequence_after_paging();

	kernel_main();

	enter_foreverloop();
}
