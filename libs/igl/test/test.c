#include <window.h>
#include <core.h>
#include <events.h>
#include <time.h>

int main()
{
	struct window *  win   = create_simple_window("Hello world!", 1000, 600);
	struct drawable *plane = create_drawable(win, 500, 500, 1, 1);
	init_events(win, EV_MOUSEBUTTON);

	while (1)
	{
		flip_display(win);
		draw_line(win->screen, 50, 50, 100, 100, 1, 0xFFFFFF);
		draw_rect(win->screen, 160, 50, 260, 100, 1, 0xFFFFFF);

		struct coord points[] = {
			{400, 400}, {500, 400}, {500, 450}, {400, 450}};
		draw_polygon(win->screen, points, 4, 0, 0xFFFFFF);

		draw_circle(win->screen, 750, 400, 50, 1, 0xFF0000);
		draw_arc(win->screen, 750, 500, 50, 0, 90, 1, 0x00FF00);

		draw_round_rect(win->screen, 160, 300, 260, 100, 10, 1, 0x0000FF);
		draw_rect(win->screen, 180, 360, 50, 50, 1, 0x80FF0000);

		draw_round_rect(plane, 0, 0, 300, 150, 20, 1, 0x333333);

		update_window(win);
	}

	for (;;)
		;
}