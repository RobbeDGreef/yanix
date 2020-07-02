#ifndef _YANIX_YANIX_INPUT_H
#define _YANIX_YANIX_INPUT_H

#define KB_KEY
#define MS_REL
#define MS_ABS

#define INP_TYPE_HID

struct input_pkt
{
	uint32_t time;
	uint16_t type;
	uint16_t code;
	uint32_t value;
};

#endif /* _YANIX_YANIX_INPUT_H */