#ifndef GUIOBJECTS_H
#define GUIOBJECTS_H

#include <stdint.h>

#define BARHEIGHT 		15
#define BARCOLOR  		0xb7b9bc
#define TOPBARHEIGHT 	20


typedef struct window_s {
	char*		name;
	uint32_t	zorder;
	uint32_t	xloc;
	uint32_t	yloc;
	uint32_t 	width;
	uint32_t	height;
	uint32_t 	bordercolor;
	uint8_t		state;
	uint8_t		moving;
	void*		buffer;
	struct window_s *next;
} window_t;


void drawWindowOutline(window_t*);
void drawTopBar(uint32_t color);
void checkWindowEvents(window_t *window);

#endif