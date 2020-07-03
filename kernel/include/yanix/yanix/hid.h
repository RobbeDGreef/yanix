#ifndef _YANIX_YANIX_HID_H
#define _YANIX_YANIX_HID_H

#include <yanix/input.h>
#include <yanix/keyboard.h>

struct mouse_move_pkt
{
	uint16_t xloc;
	uint16_t yloc;
};

struct mouse_press_pkt
{
	uint16_t press;
	uint16_t button;
};

struct input_hid_pkt
{
	struct input_pkt inp;

	int hid_type;
	union
	{
		struct keyboard_pkt    kb;
		struct mouse_press_pkt mousepress;
		struct mouse_move_pkt  mousemove;
	};
};

#endif /* _YANIX_YANIX_HID_H */