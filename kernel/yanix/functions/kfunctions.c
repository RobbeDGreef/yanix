#include <core/timer.h>
#include <cpu/io.h>
#include <debug.h>
#include <kernel.h>
#include <proc/tasking.h>

/**
 * @brief      Displays status message on display
 *
 * @param      message  The message           mm
 * @param[in]  code     The error code
 */
void message(char *message, char code)
{
	debug_printk("%s\n", message);
	if (code)
	{
		printk(KERN_INFO "[   OK   ] ");
	}
	else if (code == 1)
	{
		printk(KERN_WARNING "[ FAILED ] ");
	}
	else if (code == 2)
	{
		printk(KERN_ERR "[ ERROR  ] ");
	}
	printk("%s\n", message);
}

/**
 * @brief      Sleeps and amount of ticks
 *
 * @param[in]  amount  The amount of ticks to sleep
 */
void _tick_sleep(unsigned int amount)
{
	// NOTE: When the compiler optimizes this code it does something weird with
	// the loop where it doesn't check if the 		 variables have changed and
	// by that enter a endless loop. I fixed this by setting the volatile
	// keyword before the g_timer_tick variable in timer.c and timer.h
	unsigned int start = (unsigned int) timer_get_cur_ticks();
	while ((timer_get_cur_ticks() - start) <= amount)
		;
}

/**
 * @brief      Sleeps for a cerain amount of milliseconds
 *
 * @param[in]  milliseconds  The amount of milliseconds
 */
void sleep(unsigned int milliseconds)
{
	// this function is not accurate at all but it's not far off
	// basically made this without thinking so u know...

	// if (runningtask == 0){
	unsigned int amount = milliseconds / (timer_get_period());
	if (!amount)
		amount++;
	_tick_sleep(amount);

	/*7
	} else {
	    sleeptask(runningtask, milliseconds);
	}
	*/
}

/**
 * @brief      Shuts the system down
 *
 * @param[in]  errorcode  The errorcode
 */
void shutdown(int errorcode)
{
	clear_screenk();
	printk("Shutting down...\n%i", errorcode);
	port_byte_out(0xf4, 0x00);
}

/**
 * @todo: This is absolute crap, redo this i just made it for debugging quickly
 */
char  strbuf[9];
char *int_to_str(unsigned int val)
{
	unsigned int tmp_int      = val;
	unsigned int reversed_int = 0;
	unsigned int zeros_before = 0;
	int          i            = 0;

	while (tmp_int > 0)
	{
		reversed_int = reversed_int * 0x10 + tmp_int % 0x10;
		if ((reversed_int == 0) && ((tmp_int % 0x10) == 0))
			zeros_before++;
		tmp_int /= 0x10;
	}

	if (reversed_int == 0)
	{
		strbuf[i++] = '0';
		strbuf[i]   = 0;
		return strbuf;
	}

	while (reversed_int > 0)
	{
		if ((reversed_int % 0x10) >= 0xA)
		{
			strbuf[i++] = ('A' + ((reversed_int % 0x10) - 10));
		}
		else
		{
			strbuf[i++] = ('0' + (reversed_int % 0x10));
		}
		reversed_int /= 0x10;
	}

	for (size_t x = 0; x < zeros_before; x++)
	{
		strbuf[i++] = '0';
	}

	strbuf[i] = 0;

	return strbuf;
}