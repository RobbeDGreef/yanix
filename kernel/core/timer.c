#include <cpu/arch_timer.h>
#include <cpu/cpu.h>
#include <debug.h>
#include <kernel.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <proc/tasking.h>
#include <stdint.h>

#define TIMER_FREQ 250

/**
 * @brief      Timer value structure
 */
struct time_info
{
	volatile unsigned long ticks_since_boot;
	unsigned long          period; /* 1000 / frequency, milliseconds */
	unsigned long          frequency;
};

struct time_info *timer_info;

/**
 * @brief      Standard timer callback
 *
 * @param      regs  The regs
 */
static void timer_callback(registers_t *regs)
{
	(void) (regs);
	timer_info->ticks_since_boot++;
	schedule();
}

/**
 * @brief      Get the period of the timer
 *
 * @return     The period of the timer
 */
unsigned long timer_get_period()
{
	return timer_info->period;
}

/**
 * @brief      Get the timer frequency
 *
 * @return     The timer frequency
 */
unsigned long timer_get_frequency()
{
	return timer_info->frequency;
}

/**
 * @brief      Get the amount of ticks since last timer reset (boot)
 *
 * @return     The amount of ticks
 */
unsigned long timer_get_cur_ticks()
{
	return timer_info->ticks_since_boot;
}

/**
 * @brief      Initializes the timer.
 */
void init_timer()
{
	arch_init_timer(TIMER_FREQ, timer_callback);

	timer_info = kmalloc(sizeof(struct time_info));
	memset(timer_info, 0, sizeof(struct time_info));

	timer_info->frequency = arch_timer_get_frequency();
	timer_info->period    = 1000 / arch_timer_get_frequency();
}
