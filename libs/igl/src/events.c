#include <events.h>
#include <core.h>

struct window;

struct event_queue
{
	Event *buffer;
	int    size;
	int    maxsize;
};

struct event_queue *equeue = NULL;

void expand_queue()
{
	Event *buf = malloc(sizeof(Event) * equeue->maxsize * 2);
	memcpy(buf, equeue->buffer, sizeof(Event) * equeue->maxsize);
	free(equeue->buffer);

	equeue->buffer = buf;
	equeue->maxsize *= 2;
}

void push_event(Event *event)
{
	printf("pushed event %i\n", event->type);
	if (equeue->maxsize == equeue->size)
		expand_queue();

	memcpy(&equeue->buffer[equeue->size++], event, sizeof(Event));
}

void pop_event()
{
	equeue->size--;
	memcpy(equeue->buffer, &equeue->buffer[1], sizeof(Event) * equeue->size);
}

void _push_mouse_event(int x, int y, int pressed, int button)
{
	Event ev;
	ev.type            = EV_MOUSEBUTTON;
	ev.mbutton.x       = x;
	ev.mbutton.y       = y;
	ev.mbutton.button  = button;
	ev.mbutton.pressed = pressed;

	push_event(&ev);
}

struct event_queue *event_queue_create()
{
	struct event_queue *q;
	q          = malloc(sizeof(struct event_queue));
	q->buffer  = malloc(sizeof(Event) * EVENT_QUEUE_START_SIZE);
	q->size    = 0;
	q->maxsize = EVENT_QUEUE_START_SIZE;

	return q;
}

void init_events(struct window *win, int requests)
{
	if (equeue)
		return;

	equeue = event_queue_create();
	INIT_EVENTS(win, requests);
}

int read_event(struct window *win, Event *ev)
{
	if (equeue->size)
	{
		memcpy(ev, equeue->buffer, sizeof(Event));
		pop_event();
		return 1;
	}

	return 0;
}