#include <cpu/timer.h>
#include <cpu/isr.h>

#include <drivers/video/videoText.h>
#include <drivers/io/io.h>

#include <lib/function.h>
#include <proc/tasking.h>

volatile unsigned long g_timer_tick;
unsigned long g_timer_frequency;

/**
 * @brief      Timer callback (gets fired every time the timer interrupts)
 *
 * @param      regs  The registers pushed
 */
static void timer_callback(registers_t *regs)
{
    g_timer_tick++;
    UNUSED(regs);

    // call scheduler
    schedule();
}

/**
 * @brief      Initializes the timer
 *
 * @param[in]  freq  The frequency the timer should be set to
 */
void init_timer(unsigned long freq)
{
    g_timer_frequency = freq;
    register_interrupt_handler(IRQ0, timer_callback);

    uint32_t divisor = 1193180 / freq;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

