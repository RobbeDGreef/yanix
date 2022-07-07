#ifndef _LIBGRAPHIC_WINDOW_H
#define _LIBGRAPHIC_WINDOW_H

#include <switch.h>

struct window
{
	char *       title;      /* The title of the window frame */
	int          width;      /* The width of the window in pixels */
	int          height;     /* The height of the window in pixels */
	int          x;          /* The x location the window is spawned to */
	int          y;          /* The y location the window is spawned to */
	unsigned int foreground; /* The foreground of the window */
	unsigned int background; /* The background of the window */

	struct drawable *screen;

	IDPL_WININFO idpl; /* The independant layer info */
};

struct drawable *create_drawable(struct window *window, int x, int y,
                                 float scale_x, float scale_y);
struct window *  create_simple_window(char *name, int width, int height);
void             update_window(struct window *window);
void             drawcolor_set(struct drawable *draw, int color);
void draw_line(struct drawable *draw, int x1, int y1, int x2, int y2,
               int thickness, int color);
void draw_rect(struct drawable *draw, int x1, int y1, int width, int height,
               int fill, int color);
void draw_circle(struct drawable *draw, int x, int y, int radius, int fill,
                 int color);
void draw_polygon(struct drawable *draw, struct coord *points, int amount,
                  int fill, int color);

void draw_arc(struct drawable *draw, int x, int y, int radius, int start,
              int end, int fill, int color);

void draw_round_rect(struct drawable *draw, int x, int y, int width, int height,
                     int radius, int fill, int color);

void init_events(struct window *win, int requests);

void flip_display(struct window *win);

#endif /* _LIBGRAPHIC_WINDOW_H */