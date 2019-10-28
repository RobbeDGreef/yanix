#include <cpu/io.h>
#include <cpu/cpu.h>
#include <cpu/isr.h>

#define PIT_PORT_DATA_0     0x40
#define PIT_PORT_MODE_CMD   0x43
#define PIT_INPUT_FREQ      1193180
#define PIT_SET_FREQ_CMD    0x36

uint32_t g_pit_frequency;

/**
 * @brief      Returns the current frequency of the pit.
 *
 * @return     The current frequency of the pit.
 */
uint32_t pit_get_freq()
{
    return g_pit_frequency;
}

/**
 * @brief      Initializes the timer
 *
 * @param[in]  freq  The frequency the timer should be set to
 */
void init_pit(uint32_t freq, isr_callback_t callback)
{
    /* Set the interrupt handler to the appropriate callback */
    arch_register_interrupt_handler(IRQ0, callback);

    /* Calculate the necessary values */
    uint32_t divisor = PIT_INPUT_FREQ / freq;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    /* Send the set pit frequency command */
    port_byte_out(PIT_PORT_MODE_CMD, PIT_SET_FREQ_CMD);
    
    /* Write the new frequency */
    port_byte_out(PIT_PORT_DATA_0, low);
    port_byte_out(PIT_PORT_DATA_0, high);

    /* Save requested frequency */
    g_pit_frequency = freq;
}

