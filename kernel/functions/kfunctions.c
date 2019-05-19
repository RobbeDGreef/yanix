#include <drivers/video/videoText.h>
#include <drivers/io/io.h>
#include <cpu/timer.h>
#include <proc/tasking.h>

#include <config/kconfig.h>

extern task_t *g_runningtask;

/**
 * @brief      Displays status message on display
 *
 * @param      message  The message
 * @param[in]  code     The error code
 */
void message(char *message, char code)
{
	if (code){
		print_color("[   OK   ] ", LGREEN_ON_BLACK);
	} else if (code == 1) {
		print_color("[ FAILED ] ", LRED_ON_BLACK);
	} else if (code == 2) {
		print_color("[ ERROR  ] ", LRED_ON_BLACK);
	}
	print(message); print("\n");
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
	unsigned int start = (unsigned int) g_timer_tick;
	while ((g_timer_tick - start) <= amount);
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
	    unsigned int amount = milliseconds/(1000/g_timer_frequency);
	    _tick_sleep(amount);

    /*
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
    clear_screen();
    print("Shutting down... ");
    print_int(errorcode);
    port_byte_out(0xf4, 0x00);
}
