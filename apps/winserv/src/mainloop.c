#include <renderer.h>
#include <core.h>

int g_runningyws = 0;

void mainloop_start()
{
	g_runningyws = 1;

	while (g_runningyws)
	{
		renderer_render();
	}
}
