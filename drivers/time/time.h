#ifndef _TIME_H
#define _TIME_H

struct time_s {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_day;
	int tm_mon;
	int tm_year;
	int tm_wdag;
	int tm_yday;
	int tm_isdst;
};

/**
 * @brief      Initialises the time func
 */
void init_time();

/**
 * @brief      Gets the time.
 *
 * @return     The time.
 */
struct time_s *get_time();

#endif