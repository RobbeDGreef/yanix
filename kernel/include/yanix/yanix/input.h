#ifndef _YANIX_YANIX_INPUT_H
#define _YANIX_YANIX_INPUT_H

#include <sys/types.h>
#include <yanix/keyboard.h>

#define KB_KEY 1

#define MS_ABS 1
#define MS_REL 2

#define INP_INIT {.time=0, .type=0}
struct input_pkt
{
	uint32_t time;
	uint16_t type;
};

#define INP_KB_INIT(sc, press, trans) {{.time=0, .type=KB_KEY}, {.scancode=sc, .mode=press, .key=trans}}

/* Keyboard packet wrapper */
struct input_kb_pkt
{
	struct input_pkt inp;
	struct kb_pkt 	 kb;
};

#endif /* _YANIX_YANIX_INPUT_H */