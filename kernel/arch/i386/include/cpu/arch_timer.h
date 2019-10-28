#ifndef ARCH_TIMER_H
#define ARCH_TIMER_H

#include <cpu/cpu.h>
#include <stdint.h>

#define TIMER_FREQ 250

/**
 * @brief      Initialises the architechture dependand timer
 *
 * @param[in]  frequency  The frequency
 * @param[in]  callback   The callback
 */
void arch_init_timer(unsigned long frequency, isr_callback_t callback);

/**
 * @brief      Returns the timer frequency
 *
 * @return     The timer frequency.
 */
uint32_t arch_timer_get_frequency();

#endif