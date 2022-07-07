#include <core.h>
#include <window.h>
#include <events.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

unsigned long _create_color(struct window *win, int color)
{
	return color;

	XColor xcolor;
	xcolor.flags = DoRed | DoGreen | DoBlue;
	xcolor.red   = color >> 16 & 0xFF;
	xcolor.green = (color >> 8) & 0xFF;
	xcolor.green = color & 0xFF;
	XAllocColor(win->idpl.display,
	            XDefaultColormap(win->idpl.display, win->idpl.win), &xcolor);

	return xcolor.pixel;
}

int x11_create_window(struct window *win)
{
	win->idpl.display = XOpenDisplay(NULL);

	if (win->idpl.display == NULL)
		return -1;

	win->idpl.screen = DefaultScreen(win->idpl.display);

	/* These are just to create shorter function parameter blocks */
	Display *d = win->idpl.display;
	int      s = win->idpl.screen;

	win->idpl.win =
		XCreateSimpleWindow(d, RootWindow(d, s), win->x, win->y, win->width,
	                        win->height, 1, win->foreground, win->background);

	XSetStandardProperties(win->idpl.display, win->idpl.win, win->title,
	                       win->title, None, NULL, 0, NULL);

	win->idpl.gc = XCreateGC(win->idpl.display, win->idpl.win, 0, 0);

	XSelectInput(d, win->idpl.win,
	             ExposureMask | KeyPressMask | ButtonPressMask
	                 | ButtonReleaseMask);
	XMapWindow(d, win->idpl.win);
	return 0;
}

int x11_update_window(struct window *win)
{
	if (!XPending(win->idpl.display))
		return 0;

	XEvent *event = &win->idpl.event;
	XNextEvent(win->idpl.display, event);

	switch (win->idpl.event.type)
	{
	case ButtonRelease:
	case ButtonPress:
		{
			XButtonEvent *b = &event->xbutton;
			_push_mouse_event(b->x, b->y, (b->type == 4) ? 1 : 0, b->button);
			return 0;
		}

	case Expose:
		printf("expose\n");
		break;
	default:
		printf("Event received\n");
	}
}

void x11_draw_line(struct window *win, int x1, int y1, int x2, int y2, int t)
{
	if (t == 1)
		XDrawLine(win->idpl.display, win->idpl.win, win->idpl.gc, x1, y1, x2,
		          y2);
	else
	{
		perror("Not implemented");
	}
}

void x11_drawcolor_set(struct window *win, int color)
{
	color = _create_color(win, color);
	XSetForeground(win->idpl.display, win->idpl.gc, color);
}

void x11_draw_rect(struct window *win, int x1, int y1, int width, int height,
                   int fill)
{
	if (fill)
		XFillRectangle(win->idpl.display, win->idpl.win, win->idpl.gc, x1, y1,
		               width, height);
	else
		XDrawRectangle(win->idpl.display, win->idpl.win, win->idpl.gc, x1, y1,
		               width, height);
}

void x11_draw_circle(struct window *win, int x, int y, int radius, int fill)
{
	x -= radius;
	y -= radius;

	radius *= 2;
	if (fill)
		XFillArc(win->idpl.display, win->idpl.win, win->idpl.gc, x, y, radius,
		         radius, 0, 360 * 64);
	else
		XDrawArc(win->idpl.display, win->idpl.win, win->idpl.gc, x, y, radius,
		         radius, 0, 360 * 64);
}

void x11_draw_poly(struct window *win, struct coord *points, int amount,
                   int fill)
{
	if (fill)
		XFillPolygon(win->idpl.display, win->idpl.win, win->idpl.gc,
		             (XPoint *) points, amount, Complex, CoordModeOrigin);
	else
	{
		XPoint *newpts = malloc(sizeof(XPoint) * amount + 1);
		memcpy(newpts, points, sizeof(XPoint) * amount);
		newpts[amount] = newpts[0];
		XDrawLines(win->idpl.display, win->idpl.win, win->idpl.gc, newpts,
		           amount + 1, CoordModeOrigin);
		free(newpts);
	}
}

void x11_draw_arc(struct window *win, int x, int y, int radius, int start,
                  int end, int fill)
{
	x -= radius;
	y -= radius;

	radius *= 2;
	if (fill)
		XFillArc(win->idpl.display, win->idpl.win, win->idpl.gc, x, y, radius,
		         radius, start * 64, end * 64);
	else
		XDrawArc(win->idpl.display, win->idpl.win, win->idpl.gc, x, y, radius,
		         radius, start * 64, end * 64);
}

void x11_select_events(struct window *win, int request)
{
	int select = ExposureMask;

	if (request & EV_MOUSEBUTTON)
		select |= ButtonPressMask;

	XSelectInput(win->idpl.display, win->idpl.win, select);
}