#ifndef _YANIX_YANIX_INPUT_H
#define _YANIX_YANIX_INPUT_H

#include <sys/types.h>
#include <yanix/keyboard.h>

#define KB_KEY 1

#define MS_ABS 1
#define MS_REL 2

#define YINP_TYPE_KB		0
#define YINP_TYPE_MS_MOVE 	1
#define YINP_TYPE_MS_PRESS 	2 

/* Yanix input packet */
#define INP_INIT {.time=0, .type=0}
struct yinp_pkt
{
	time_t 	time;	/* The timestamp of the packet */
	size_t 	size;	/* The size of the payload */
	int 	type;	/* The type of payload */
};

#define INP_KB_INIT(sc, press, trans) {{.time=0, .type=KB_KEY}, {.scancode=sc, .mode=press, .key=trans}}

/* Keyboard packet wrapper (SHOULD NOT BE USED) */

struct yinp_kb_pkt
{
	struct yinp_pkt  inp;	/* The input packet header */
	struct ykb_pkt 	 kb;	
};

#endif /* _YANIX_YANIX_INPUT_H */
