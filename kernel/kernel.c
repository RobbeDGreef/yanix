#include <drivers/video/videoText.h>		// text driver (vga & vesa automatically selected), (init)
#include <drivers/video/vesaText.h>  		// vesa text mode 				(init)
#include <drivers/keyboard/ps2.h>			// ps2 keyboard 				(init)
#include <drivers/mouse/ps2.h>				// ps2 mouse (for gui)			(init)
#include <drivers/vfs/vfs.h>				// virtual file system			(init)
#include <fs/filedescriptor.h>		 		// file descriptor system 		(init)
#include <cpu/timer.h>						// cpu timer 					(init)
#include <cpu/isr.h>						// interrupt service routines 	(init)
#include <cpu/gdt.h>						// global descriptor table		(init)
#include <drivers/ramdisk/ramdisk.h>		// ramdisk 						(init)
#include <proc/tasking.h>					// tasking 						(init)
#include <mm/paging.h>						// paging 						(init)
#include <mm/heap.h>						// kernel heap 					(init)
#include <proc/syscall.h>					// the system calls 			(init)
#include <kernel/stack/stack.h>				// stack functions 				(kernel init)

#include <fs/fs.h>							// the filesystem api

#include <config/kconfig.h>					// kernel configuration file
#include <kernel/functions/kfunctions.h>	// kernel functions

#include <stdint.h>

#include <lib/function.h>
#include <lib/string/string.h>

#include <gui/gui.h>
#include <fs/ext2/ext2.h>
#include <kernel/shell/shell.h>				// to start a shell on TTY1


extern task_t *g_starttask;
extern task_t *g_runningtask;
extern uint32_t get_eip();


void bootsequence(uint32_t stack, uint32_t code_gdt, uint32_t data_gdt){
	// initalize the stack
	init_stack(stack);

	UNUSED(code_gdt);UNUSED(data_gdt);

	clear_screen();
	init_vesatext();
	message("Vesatext mode initialized", 1);
	print("initial stack: "); print_hex(stack);

	message("Booting sequence started", 1);
	init_descriptor_tables();
	message("Global descriptor table initialized", 1);
	isr_install();
	message("Interrupt service routines set", 1);
	asm volatile ("sti");
	message("Interrupt flag set, now listening for interrupts", 1);
	init_timer(1000); // hz
	message("Timer initialized", 1);
	init_keyboard();
	message("Keyboard initialized", 1);
	init_paging();
	message("Paging intialized", 1);
}

void bootsequence_paging()
{
	int ret = 1;
	message("bootsequence paging started", 1);

	// initialize the kernel heap
	init_kheap();
	message("Heap initialized", 1);

	// initialize the tasking module
	init_tasking();
	g_starttask = g_runningtask;		// i don't know why i need to do this here but if i don't it won't work
	message("Tasking initialized", 1);
	
	// initialize the cursor (NEEDS FIX)
	//init_vesa_cursor();
	//message("Cursor initialized", 1);
	
	// initialize ramdisk 
	init_ramdisk(0x10000, 64*1024);
	message("Ramdisk initialized", 1);
	
	// initialize the virtual filesystem
	init_vfs();
	message("VFS initialized", 1);

	// initialize the file descriptor system
	ret = init_filedescriptors();
	message("File descriptors initialized", ret);

	// initalize the mouse driver
	init_mouse();
	message("Mouse initialized", 1);
	
	// initialize the system calls
	init_syscalls();
	message("Syscalls initialized", 1);
	
	// sleep to read the messages
	sleep(TIMEAFTERBOOT);

	// cleanup
	//clear_screen();
}


void kernel_main()
{
	print("kernel booted\n");

	print("running...\n");

	//init_shell();
	//jump_usermode();
	
	//sleep(5000);
	//syscall_print("hello world!");

}

void enter_foreverloop()
{
	// execute halt instruction forever
	print("\nEnd of main kernel loop, exiting...\n");
	while (1) {
		asm volatile ("hlt");
	}
}

void _enter(uint32_t stack, uint32_t code_gdt, uint32_t data_gdt)
{
	// enter function for kernel
	// starts bootsequence
	bootsequence(stack, code_gdt, data_gdt);
	
	// maps the stack to the wanted location
	init_paging_stack();
	message("Successfully remapped kernel stack to disired location", 1);

	// paging part of bootsequence
	bootsequence_paging();
	
	// jump to main kernel code
	kernel_main();

	// after main kernel code enter a forever loop of halt instuctions
	enter_foreverloop();
}

// todo:	create TTY system so that multiple workspaces become possible 
//			this will change nothing about the print()'s we would just bind a process to a tty 
