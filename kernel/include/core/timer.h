#ifndef TIMER_H
#define TIMER_H

/**
 * @brief      Get the timer frequency
 *
 * @return     The timer frequency
 */
unsigned long timer_get_frequency();

/**
 * @brief      Initializes the timer.
 */
void init_timer();


/**
 * @brief      Get the amount of ticks since last timer reset (boot)
 *
 * @return     The amount of ticks 
 */
unsigned long timer_get_cur_ticks();


/**
 * @brief      Get the period of the timer 
 *
 * @return     The period of the timer
 */
unsigned long timer_get_period();

#endif