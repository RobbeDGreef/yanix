#include <drivers/video/graphics.h>
#include <gui/guiObjects.h>
#include <drivers/mouse/ps2.h>

extern int32_t mouse_xloc, mouse_yloc, mouse_xrel, mouse_yrel;
extern uint8_t mouse_lbut, mouse_lheld;

void checkWindowEvents(window_t *window){
	if (window->moving){
		window->xloc = mouse_xloc - (window->width / 2);
		window->yloc = mouse_yloc + (BARHEIGHT/2);
		if (!mouse_lheld){
			window->moving = 0;
		}
		return;
	}

	if (mouse_lheld){
		if (window->xloc < (uint32_t) mouse_xloc && (uint32_t) mouse_xloc < (window->xloc + window->width)){
			if ((window->yloc-BARHEIGHT) < (uint32_t) mouse_yloc && (uint32_t) mouse_yloc < window->xloc){
				window->moving = 1;
			}
		}
	}
}

void drawWindowOutline(window_t *window){
	drawrect_RAW(window->xloc, window->yloc-BARHEIGHT, window->width, BARHEIGHT, window->bordercolor, window->buffer);
	drawrect_RAW(window->xloc,window->yloc,window->width,window->height,0xffffff, window->buffer);
	drawtext(window->name, window->xloc, window->yloc - BARHEIGHT, 0x000000);
}

void drawTopBar(uint32_t color){
	drawrect(0,0,WIDTH, TOPBARHEIGHT, color);
}