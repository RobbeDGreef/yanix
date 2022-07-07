#include <core.h>
#include <drawable.h>
#include <window.h>

struct Coord;

struct drawable *create_drawable(struct window *window, int x, int y,
                                 float scale_x, float scale_y)
{
	struct drawable *draw = malloc(sizeof(struct drawable));
	draw->window          = window;
	draw->x               = x;
	draw->y               = y;
	draw->scale_x         = scale_x;
	draw->scale_y         = scale_y;

	return draw;
}

struct window *create_simple_window(char *name, int width, int height)
{
	struct window *win = malloc(sizeof(struct window));
	memset(win, 0, sizeof(struct window));
	win->title      = strdup(name);
	win->width      = width;
	win->height     = height;
	win->foreground = 0x3b5689;
	win->background = 0x424242;
	win->screen     = create_drawable(win, 0, 0, 1.0, 1.0);

	if (CREATE_WINDOW(win) == -1)
		return NULL;

	return win;
}

void update_window(struct window *window)
{
	UPDATE_WINDOW(window);
}

void drawcolor_set(struct drawable *draw, int color)
{
	if (draw->window->foreground != color)
	{
		draw->window->foreground = color;
		DRAWCOLOR_SET(draw->window, color);
	}
}

void draw_line(struct drawable *draw, int x1, int y1, int x2, int y2, int thick,
               int color)
{
	drawcolor_set(draw, color);
	DRAW_LINE(draw->window, locTrans(draw, x1), locTrans(draw, y1),
	          locTrans(draw, x2), locTrans(draw, y2), thick);
}

void draw_rect(struct drawable *draw, int x, int y, int width, int height,
               int fill, int color)
{
	drawcolor_set(draw, color);
	DRAW_RECT(draw->window, locTrans(draw, x), locTrans(draw, y), width, height,
	          fill);
}

void draw_circle(struct drawable *draw, int x, int y, int radius, int fill,
                 int color)
{
	drawcolor_set(draw, color);
	DRAW_CIRCLE(draw->window, locTrans(draw, x), locTrans(draw, y), radius,
	            fill);
}

void draw_polygon(struct drawable *draw, struct coord *points, int amount,
                  int fill, int color)
{
	drawcolor_set(draw, color);
	DRAW_POLYGON(draw->window, coordTrans(draw, points, amount), amount, fill);
}

void draw_arc(struct drawable *draw, int x, int y, int radius, int start,
              int end, int fill, int color)
{
	drawcolor_set(draw, color);
	DRAW_ARC(draw->window, locTrans(draw, x), locTrans(draw, y), radius, start,
	         end, fill);
}

void draw_round_rect(struct drawable *draw, int x, int y, int width, int height,
                     int radius, int fill, int color)
{
	drawcolor_set(draw, color);

	draw_arc(draw, x + radius, y + radius, radius, 90, 90, fill, color);
	draw_arc(draw, x + radius, y + height - radius, radius, -90, -90, fill,
	         color);
	draw_arc(draw, x + width - radius, y + radius, radius, 0, 90, fill, color);
	draw_arc(draw, x + width - radius, y + height - radius, radius, 0, -90,
	         fill, color);

	if (fill)
	{
		draw_rect(draw, x + radius, y, width - radius * 2, radius, 1, color);
		draw_rect(draw, x, y + radius, width, height - radius * 2, 1, color);
		draw_rect(draw, x + radius, y + height - radius, width - radius * 2,
		          radius, 1, color);
	}
	else
	{
		printf("Not implemented\n");
	}
}

void flip_display(struct window *win)
{
	int fg = win->foreground;
	drawcolor_set(win->screen, win->background);
	CLEAR_DISPLAY(win);
	win->foreground = fg;
	drawcolor_set(win->screen, fg);
}