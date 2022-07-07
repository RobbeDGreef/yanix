#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <eclib/vector.h>

#include <yanix/input.h>
#include "event.h"

int _g_le_running = 0;

/* svec is an abriviation for simple vector */
define_vector(void*, svec);

svec g_kb_list;
svec g_ms_move_list;
svec g_ms_press_list;

static void notify_list(svec list, struct le_event *ev)
{
	int i;
	
	printf("Trying to notify?i %i\n", list->size);

	if (list->size)
	{
		void (*ptr)(struct le_event*) = svec_get(list, 0);
		ptr(ev);
	}

#if 0
	while ((i = svec_iter(list)) != -1)
	{
		void (*ptr)(struct le_event *) = svec_get(list, i);
		printf("notifing %x\n", ptr);
		ptr(ev);
	}
#endif

}	

static void handle_unknown_pkt(int eventdev, size_t size)
{
	/* @todo: this is baaaddd, but I don't know how to handle this properly any other way */
	void *garbage = malloc(size);
	read(eventdev, garbage, size);
	free(garbage);
}

static void evdev_parse_kb(int eventdev, struct yinp_pkt *inp)
{
	struct ykb_pkt kbev;
	if (read(eventdev, &kbev, inp->size) != inp->size)
	{
		printf("Failed\n");
		return;
	}

	struct le_event *ev = malloc(sizeof(struct le_event));
	ev->type = LE_EVTYPE_KB;
	ev->timestamp = inp->time;
	
	ev->kb.scancode = kbev.scancode;
	ev->kb.translated = kbev.key;
	ev->kb.pressmode = kbev.mode;

	notify_list(g_kb_list, ev);
	free(ev);
	printf("ret\n");
}

static void evdev_parse_msmove(int eventdev, struct yinp_pkt *inp)
{
}	

static void evdev_parse_mspress(int eventdev, struct yinp_pkt *inp)
{
}

static void evdev_parseinp_pkt(int eventdev, struct yinp_pkt *ev)
{
	switch (ev->type)
	{
	case YINP_TYPE_KB:
		evdev_parse_kb(eventdev, ev);	
		break;
	case YINP_TYPE_MS_MOVE:
		evdev_parse_msmove(eventdev, ev);
		break;
	case YINP_TYPE_MS_PRESS:
		evdev_parse_mspress(eventdev, ev);
		break;
	default:
		/* Woopsie */
		handle_unknown_pkt(eventdev, ev->size);
	}
	printf("huh?\n");
}

static void evdev_handler(int eventdev)
{
	struct yinp_pkt ev;

	while (_g_le_running)
	{
		printf("Loop\n");
		if (read(eventdev, &ev, sizeof(ev)) == sizeof(ev))
		{
			printf("read some stuff\n");
			evdev_parseinp_pkt(eventdev, &ev);
			printf("that was it\n");
		}
		else
		{
			printf("[DEBUG]: read read arbitrary amount of bytes\n");
		}
	}
}

int libevent_hook_kb(void (*ptr)(struct le_event*))
{
	if (_g_le_running == 0)
		return -1;
	
	svec_push(g_kb_list, ptr);
	printf("pushed %x\n", ptr);
}

static void spawnhandler()
{
	int fd = open("/dev/input/event0", O_RDONLY);
	if (fd == -1)
	{
		printf("Nah g\n");
		return;
	}
	evdev_handler(fd);
}

int libevent_init()
{
	g_kb_list = svec_create();
	g_ms_move_list = svec_create();
	g_ms_press_list = svec_create();

	_g_le_running = 1;

	if (fork() == 0)
	{
		spawnhandler();
		printf("should never reach\n");
	}
}

int libevent_exit()
{
	_g_le_running = 0;
}
