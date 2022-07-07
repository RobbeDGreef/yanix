#ifndef _LIBGRAPHIC_EVENTS_H
#define _LIBGRAPHIC_EVENTS_H

#define EVENT_QUEUE_START_SIZE 32

typedef struct mbutton_event
{
	int x, y;
	int button;
	int pressed;
} MButtonEvent;

typedef struct event
{
	int type;
	union
	{
		MButtonEvent mbutton;
	};
} Event;

#define EV_MOUSEBUTTON 1

struct window;

void _push_mouse_event(int x, int y, int pressed, int button);
int  read_event(struct window *win, Event *ev);

#endif /* _LIBGRAPHIC_EVENTS_H */