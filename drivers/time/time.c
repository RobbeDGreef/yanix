#include <drivers/cmos/cmos.h>
#include <drivers/time/time.h>
#include <mm/heap.h>

//struct time_s *g_time;
time_t 		 g_epoch_seconds;	// seconds
unsigned int g_epoch_delta;		// milliseconds

void time_add_delta(unsigned int milliseconds) 
{
	g_epoch_delta += milliseconds;
	if (g_epoch_delta > 1000) {
		g_epoch_seconds++;
		g_epoch_delta = 0;
	}
}

/**
 * @brief      Gets the time.
 *
 * @return     The time.
 */
time_t time(time_t *second) 
{
	if (second != 0) {
		*second = g_epoch_seconds;
		return 0;
	} else {
		return g_epoch_seconds;
	}
}

#include <drivers/video/videoText.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

/**
 * @brief      Initialises the time func
 */
void init_time()
{
	struct time_s *g_time = (struct time_s*) kmalloc(sizeof(struct time_s));

	g_time->tm_sec = cmos_read(0);
	g_time->tm_min = cmos_read(2);
	g_time->tm_hour = cmos_read(4);
	g_time->tm_mday = cmos_read(7);
	g_time->tm_mon = cmos_read(8)-1;
	g_time->tm_year = cmos_read(9);

	g_time->tm_sec = bcd_to_bin(g_time->tm_sec);
	g_time->tm_min = bcd_to_bin(g_time->tm_min);
	g_time->tm_hour = bcd_to_bin(g_time->tm_hour);
	g_time->tm_mday = bcd_to_bin(g_time->tm_mday);
	g_time->tm_mon = bcd_to_bin(g_time->tm_mon);
	g_time->tm_year = bcd_to_bin(g_time->tm_year);

	// now calculate the epoch
	// @todo: calculate (maybe this is c lib stuff not sure)
	g_epoch_seconds = 0;
	g_epoch_seconds = 0;
}

