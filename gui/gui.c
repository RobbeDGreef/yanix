#include <gui/gui.h>
#include <gui/guiObjects.h>

#include <drivers/video/graphics.h>	 		// graphic functions	  (api)
#include <drivers/mouse/ps2.h>				// the mouse functions    (api)
#include <drivers/keyboard/ps2.h>			// the keyboard functions (api)
#include <kernel/functions/kfunctions.h>  	// sleep				  (api)
#include <mm/heap.h>						// heap	
#include <proc/tasking.h>					// tasking 				  (api)
#include <lib/string/string.h>				// memory functions 	  (api)

#include <drivers/video/videoText.h>

uint8_t LOOPING = 0;
uint8_t button;

window_t *windows;


extern int32_t mouse_xloc, mouse_yloc;
extern uint8_t mouse_lbut, mouse_lheld;

static void keyboardcallback(uint8_t character){
	button = character;
}



void fillbackground(uint32_t color){
	drawrect_RAW(0,0,WIDTH,HEIGHT,color, (uint8_t*) BACKGROUNDMEMORY);
}


void fillbuffer(uint32_t *dest, uint32_t max, uint32_t color){
	for (uint32_t i = 0; i < max; i+=3){
		dest[i] = color & 255;
		dest[i+1] = (color >> 8) & 255;
		dest[i+2] = (color >> 16) & 255;
	}
}

void flip_display(){
	while (1){
		refresh_display();
	}
}


void init_gui(){
	// get all hooks
	print("fuck your mom");
	register_keyboard_hook(keyboardcallback);
	//init_graphics();
	uint32_t backgroundcolor = 0x4286f4;
	
	//clear_graphics();

	window_t *w = (window_t*) kmalloc(sizeof(window_t));
	w->name = "test window";
	w->xloc = 50;
	w->yloc = 50;
	w->width = 300;
	w->height = 500;
	w->bordercolor = BARCOLOR;
	w->moving = 0;
	//w->buffer = (void*) WINDOWMEMORY;
	w->buffer = (void*) ZBuffer;

	windows = w;

	window_t *currentwindow;
	LOOPING = 1;

	
	fillbackground(backgroundcolor);

	const uint32_t MOUSEBUFFERMAX = 100*3;
	uint32_t MOUSEBUFFER = (uint32_t) kmalloc(MOUSEBUFFERMAX);
	fillbuffer((uint32_t*) MOUSEBUFFER, MOUSEBUFFERMAX, 0xff00ff);

	//fork_func_advanced(flip_display, "displayflip", 2, 10, 0);

	while (LOOPING){
		// clear screen
		// render everything
		//print("fuck your mom");
		memcpy((void*) ZBuffer, (void*) BACKGROUNDMEMORY, MAXBUFFER);
		currentwindow = windows;
		while (currentwindow != 0){
			checkWindowEvents(currentwindow);
			drawWindowOutline(windows);
			//memcpy_fast(currentwindow->buffer, (void*) ZBuffer, MAXBUFFER);		// BUG: we memcpy a buffer that is half empty into the zbuffer now
			//emcpy_fast_location(currentwindow->buffer, (uint32_t*) ZBuffer, currentwindow->xloc, currentwindow->yloc, currentwindow->width, currentwindow->height);
			refresh_rect(currentwindow->xloc, currentwindow->yloc, currentwindow->width, currentwindow->height);
			currentwindow = currentwindow->next;
		}

		drawTopBar(0x303030);
		drawrect(mouse_xloc, mouse_yloc, 10,10,0xff00ff);
		//memcpy_fast_location((void*) MOUSEBUFFER, (void*) ZBuffer, mouse_xloc, mouse_yloc, 10, 10);
		refresh_display();
	}
}