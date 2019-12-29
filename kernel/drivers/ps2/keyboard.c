#include <cpu/io.h>
#include <cpu/cpu.h>
#include <libk/string.h>
#include <libk/function.h>

#include <fs/vfs.h>
#include <drivers/ps2/keyboard.h>

#include <config/belgian.h>
#include <config/font.h>

/* @todo: redo keyboard driver */

#define PS2_DATA_PORT 0x60

#define LSHIFT_D    0x2a
#define LSHIFT_U    0xaa

#define CLOCK       0x3a

#define RSHIFT_D    0x36
#define RSHIFT_U    0xb6

#define ALTGR_D     22456
#define ALTGR_U     224184


unsigned char get_key_from_scancode(uint8_t scancode, int, int);
int shift_currently_enabled = 0;
int ALTGRPRESSED = 0;

#include <debug.h>
/**
 * @brief      The default keyboard callback, writes the pressed character to the stdin
 *
 * @param      regs  The regs
 */
static void keyboard_callback(registers_t *regs)
{
	UNUSED(regs);

	unsigned char scancode = port_byte_in(PS2_DATA_PORT);
	if (scancode == LSHIFT_D || scancode == RSHIFT_D)
		shift_currently_enabled = 1;
	
	else if (scancode == LSHIFT_U || scancode == RSHIFT_U)
		shift_currently_enabled = 0;
	
	else if (scancode == CLOCK)
		shift_currently_enabled = !shift_currently_enabled;

	else 
	{
		unsigned char c = get_key_from_scancode(scancode, shift_currently_enabled, 0);
		vfs_write_fd(0, &c, 1);
	}
}

/**
 * @brief      Initialize keyboard function
 */
void init_keyboard()
{
    arch_register_interrupt_handler(IRQ1, &keyboard_callback);
}

/**
 * @brief      Gets a key from scancode.
 *
 * @param[in]  scancode      The scancode
 * @param[in]  ShiftPressed  Is shift pressed
 * @param[in]  AltGrPressed  Is altgr pressed
 *
 * @return     The key.
 */
unsigned char get_key_from_scancode(unsigned char scancode, int shift_enabled, int altgr_enabled)
{
    if (scancode <= KeyboardListMax){
        if (shift_enabled == 0 && altgr_enabled == 0) {
            return NormalKeyList[(int) scancode];
        }else if (shift_enabled == 1){
            return ShiftKeyList[(int) scancode];
        }else{
            return AltGrKeyList[(int) scancode];
        }
    }else{
        return 0x0;
    }
}
