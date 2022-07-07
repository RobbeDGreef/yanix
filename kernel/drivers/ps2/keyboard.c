#include <cpu/cpu.h>
#include <cpu/io.h>
#include <drivers/keyboard.h>

#define PS2_DATA_PORT 0x60

#define PS2KBD_RESET 0xFF
#include <debug.h>

static void keyboard_callback(registers_t *regs)
{
	uint8_t inb = port_byte_in(PS2_DATA_PORT);

	if (inb == 0 || inb == 0xFE || inb == 0xFF)
	{
		debug_printk("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA keyboard trouble\n");
		for(;;);
	}

	debug_printk("hihi %x\n", inb);

	send_scancode(inb);
}

int init_ps2_keyboard()
{
	port_byte_out(PS2_DATA_PORT, PS2KBD_RESET);

	arch_register_interrupt_handler(IRQ1, &keyboard_callback);
	return 0;
}
