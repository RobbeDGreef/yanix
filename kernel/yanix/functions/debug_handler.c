#include <kernel.h>
#include <cpu/cpu.h>

void debug_handler(registers_t *regs)
{
	printk("Debug handler not implemented anymore, sublime removed it :/\n");
	for(;;);
}
