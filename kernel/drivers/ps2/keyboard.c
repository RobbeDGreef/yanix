#include <cpu/io.h>
#include <cpu/cpu.h>
#include <drivers/keyboard.h>

#define PS2_DATA_PORT 0x60

static void keyboard_callback(registers_t *regs)
{
	unsigned char scancode = port_byte_in(PS2_DATA_PORT);
	send_scancode(scancode);
}

int init_ps2_keyboard()
{
    arch_register_interrupt_handler(IRQ1, &keyboard_callback);
    return 0;
}
