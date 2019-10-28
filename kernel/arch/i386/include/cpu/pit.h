#ifndef PIT_H
#define PIT_H

#include <cpu/cpu.h>
#include <stdint.h>

/**
 * @brief      Returns the current frequency of the pit.
 *
 * @return     The current frequency of the pit.
 */
uint32_t pit_get_freq();

/**
 * @brief      Initializes the timer
 *
 * @param[in]  freq  The frequency the timer should be set to
 */
void init_pit(uint32_t freq, isr_callback_t callback);

#endif