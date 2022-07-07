#ifndef _YANIX_YANIX_KEYBOARD_H
#define _YANIX_YANIX_KEYBOARD_H

#include <sys/types.h>

#define KEYMODE_PRESSED    0
#define KEYMODE_RELEASED   1
#define KEYMODE_AUTOREPEAT 2

struct ykb_pkt
{
	unsigned int scancode;   /* Scancode of pressed key */
	int			 mode;       /* Mode of press */
	int			 key; 		 /* Translated key */
};

#endif /* _YANIX_YANIX_KEYBOARD_H */
