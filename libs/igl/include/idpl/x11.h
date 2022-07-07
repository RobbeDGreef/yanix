#ifndef _IDPL_X11_H
#define _IDPL_X11_H

#include <X11/Xlib.h>

struct window;
struct coord;

struct x11_idpl_wininfo
{
	Display *display;
	Window   win;
	XEvent   event;
	int      screen;
	GC       gc;
};

int  x11_create_window(struct window *win);
int  x11_update_window(struct window *win);
void x11_drawcolor_set(struct window *win, int color);
void x11_draw_line(struct window *win, int x1, int y1, int x2, int y2, int t);
void x11_draw_rect(struct window *win, int x1, int y1, int width, int height,
                   int fill);
void x11_draw_circle(struct window *win, int x, int y, int radius, int fill);
void x11_draw_poly(struct window *win, struct coord *points, int amount,
                   int fill);

void x11_draw_arc(struct window *win, int x, int y, int radius, int start,
                  int end, int fill);

void x11_select_events(struct window *win, int request);
#endif /* _IDPL_X11_H */