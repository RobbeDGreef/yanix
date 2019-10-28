#include <cpu/io.h>
#include <core/timer.h>
#include <proc/tasking.h>
#include <kernel.h>

extern task_t *g_runningtask;

/**
 * @brief      Displays status message on display
 *
 * @param      message  The message           mm
 * @param[in]  code     The error code
 */
void message(char *message, char code)
{
	if (code){
		printk(KERN_INFO "[   OK   ] ");
	} else if (code == 1) {
		printk(KERN_WARNING "[ FAILED ] ");
	} else if (code == 2) {
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
	// NOTE: When the compiler optimizes this code it does something weird with the loop where it doesn't check if the 
	// 		 variables have changed and by that enter a endless loop. I fixed this by setting the volatile keyword 
	// 		 before the g_timer_tick variable in timer.c and timer.h 
	unsigned int start = (unsigned int) timer_get_cur_ticks();
	while ((timer_get_cur_ticks() - start) <= amount);
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
    
    //if (runningtask == 0){	
	    unsigned int amount = milliseconds/(timer_get_period());
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
