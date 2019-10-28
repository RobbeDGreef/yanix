#include <cpu/io.h>
#include <cpu/cpu.h>
#include <libk/string.h>
#include <libk/function.h>

#include <drivers/ps2/keyboard.h>

#include <config/belgian.h>
#include <config/font.h>

/* @todo: redo keyboard driver */

#define False 0
#define True 1

#define LSHIFT_D    0x2a
#define LSHIFT_U    0xaa

#define CLOCK       0x3a

#define RSHIFT_D    0x36
#define RSHIFT_U    0xb6

#define ALTGR_D     22456
#define ALTGR_U     224184

#define HOOKS		16 						// max hooks
keyhook_fpointer	keyboardhooks[HOOKS];


uint8_t getKey(uint8_t scancode, int, int);
int SHIFTPRESSED = False;
int ALTGRPRESSED = False;


/**
 * @brief      The standard keyboard callback that calls all the appropirate hooks
 *
 * @param      regs  The registers struct
 */
static void keyboard_callback(registers_t *regs)
{
	UNUSED(regs);
	uint8_t scancode = port_byte_in(0x60);
	if (scancode == LSHIFT_D || scancode == RSHIFT_D) {SHIFTPRESSED = True;}
	
	// alt gr removed because of error discussed in todo.txt

	else if (scancode == LSHIFT_U || scancode == RSHIFT_U) {SHIFTPRESSED = False;}
	else if (scancode == CLOCK){SHIFTPRESSED = !SHIFTPRESSED;}
	else{
		uint8_t k = getKey(scancode, SHIFTPRESSED, ALTGRPRESSED);
		if (k != 0){
			for (uint32_t i = 0; i < HOOKS; i++){
				if (keyboardhooks[i] != 0){
					keyboardhooks[i](k);	
				}
			}
		}
	}
}

/**
 * @brief      Initialize keyboard function
 */
void init_keyboard()
{
    arch_register_interrupt_handler(IRQ1, keyboard_callback);
    memset((uint32_t*) keyboardhooks, 0, sizeof(keyhook_fpointer)*HOOKS);
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
uint8_t getKey(uint8_t scancode, int ShiftPressed, int AltGrPressed)
{
    if (scancode <= KeyboardListMax){
        if (ShiftPressed == False && AltGrPressed == False) {
            return NormalKeyList[(int) scancode];
        }else if (ShiftPressed == True){
            return ShiftKeyList[(int) scancode];
        }else{
            return AltGrKeyList[(int) scancode];
        }
    }else{
        return 0x0;
    }
}

/**
 * @brief      Registers a keyboard hook
 *
 * @param[in]  hook  The hook registered
 */
void register_keyboard_hook(keyhook_fpointer hook)
{
	for (uint32_t i = 0; i < HOOKS; i++){
		if (keyboardhooks[i] == 0){
			keyboardhooks[i] = hook;
			return;
		}
	}
}

/**
 * @brief      Removes a keyboard hook.
 *
 * @param[in]  hook  The hook
 */
void remove_keyboard_hook(keyhook_fpointer hook)
{
	for (uint32_t i = 0; i < HOOKS; i++){
		if (keyboardhooks[i] == hook){
			keyboardhooks[i] = 0;
		}
	}
}