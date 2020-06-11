#include <lgfx.h>

int main()
{
	lgfx_init();

	unsigned long testframe[] = {0x00FF0000, 0x0000FF00, 0x000000FF,
	                             0x000000FF, 0x0000FF00, 0x00FF0000};

	lgfx_draw_frame_32((unsigned long) &testframe, 3, 2);

	/*
	int i = 0;
	while (1)
	{
	    lgfx_flip_display();
	    lgfx_draw_point_raw(i, 0xFF0000);
	    lgfx_update_display();
	    i++;
	}
	*/

	lgfx_quit();
}