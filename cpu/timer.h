#ifndef TIMER_H
#define TIMER_H

extern volatile unsigned long g_timer_tick;
extern unsigned long g_timer_frequency;

/**
 * @brief      Initializes the timer
 *
 * @param[in]  freq  The frequency the timer should be set to
 */
void init_timer(unsigned long freq);

#endif /* timer.h */
