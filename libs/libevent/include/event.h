#ifndef _LIBEVENT_EVENT_H
#define _LIBEVENT_EVENT_H

#include <sys/types.h>

#define LE_KB_PMODE_DOWN 	1	/* Key pressed down */
#define LE_KB_PMODE_UP		2	/* Key released */
#define LE_KB_PMODE_AUTORPT	3	/* Key pressed in autorepeat */

#define LE_KB_PMODE_DOWN	1	/* The button was pressed */
#define LE_KB_PMODE_UP		2	/* The button was released */

#define LE_EVTYPE_KB 1

struct le_keyevent
{
	unsigned int scancode;		/* The scancode of the key */
	unsigned int translated;	/* The translated character from the key (if possible */
	unsigned int pressmode;		/* The pressmode check LE_KB_PMODE_* for possible values */
};

struct le_mousemove
{
	unsigned short xloc;	/* The new X location of the mouse pointer */
	unsigned short yloc;	/* The new Y location of the mouse pointer */
	
	short xrel;				/* The relative change of mouse pointer X location */
	short yrel;				/* The relative change of mouse pointer Y location */
};

struct le_mousepress
{
	unsigned int button;	/* The button that was pressed on the mouse */
	unsigned int pressmode;	/* The pressmode check LE_MS_PMODE_* for possible values */
};

struct le_event
{
	int 	type;		/* The type of event see LE_EVTYPE_* */
	time_t 	timestamp;	/* The timestamp of the event */

	union
	{
		struct le_keyevent kb;				/* Keyboard event */
		struct le_mousemove mousemove;		/* Mousebutton press event */ 
		struct le_mousepress mousepress;	/* Mousemove event */
	};
};

#endif /* _LIBEVENT_EVENT_H */
