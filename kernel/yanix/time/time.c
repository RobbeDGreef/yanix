#include <core/timer.h>

int kern_uptime_ticks()
{
	return timer_get_cur_ticks();
}

int kern_uptime_milli()
{
	return timer_get_cur_ticks() * timer_get_period();
}