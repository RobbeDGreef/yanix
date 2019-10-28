#ifndef KEYBOARD_PS2_H
#define KEYBOARD_PS2_H

#include <stdint.h>

typedef void (*keyhook_fpointer)(uint8_t);

/**
 * @brief      Initialize keyboard function
 */
void init_keyboard();

/**
 * @brief      Registers a keyboard hook
 *
 * @param[in]  hook  The hook registered
 */
void register_keyboard_hook(keyhook_fpointer hook);

/**
 * @brief      Removes a keyboard hook.
 *
 * @param[in]  hook  The hook
 */
void remove_keyboard_hook(keyhook_fpointer hook);

#endif