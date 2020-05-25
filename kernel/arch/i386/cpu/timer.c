#include <cpu/cpu.h>
#include <cpu/pit.h>

/**
 * @brief      Initialises the architechture dependand timer
 *
 * @param[in]  frequency  The frequency
 * @param[in]  callback   The callback
 */
void arch_init_timer(unsigned long frequency, isr_callback_t callback)
{
	/* Initialise our pit */
	init_pit(frequency, callback);
}

/**
 * @brief      Returns the timer frequency
 *
 * @return     The timer frequency.
 */
uint32_t arch_timer_get_frequency()
{
	return pit_get_freq();
}