#include <window.h>
#include <renderer.h>
#include <core.h>

static void *create_winbuf(int w, int h)
{
	return malloc(YW_BPP * w * h);
}

struct ywindow *yws_create(char *t, int x, int y, int width, int height)
{
	struct ywindow *win = malloc(sizeof(struct ywindow));

	if (win == NULL)
		return NULL;

	win->title = t;
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;

	win->buffer = create_winbuf(width, height);
	
	if (win->buffer == NULL)
	{
		free(win);
		return NULL;
	}

	renderer_pushwin(win);

	return win;
}

struct ywindow *yws_create_simple(char *t, int w, int h)
{
	return yws_create(t, YW_DEF_X, YW_DEF_Y, w, h);
}
