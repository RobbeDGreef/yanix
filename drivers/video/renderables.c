#include <drivers/video/graphics.h>

#define True 1
#define False 0

void drawRoundedBox(unsigned int x1, unsigned int y1, unsigned int width, unsigned int height, unsigned int radius, unsigned int color){
	drawrect(x1, y1-radius, width, height+radius*2+1, color);

	drawcirclequadrant(x1,y1,radius, True, 2, color);
	drawcirclequadrant(x1+width,y1,radius, True, 1, color);
	drawcirclequadrant(x1+width,y1+height,radius, True, 4, color);
	drawcirclequadrant(x1,y1+height,radius, True, 3, color);

	drawrect(x1-radius, y1, radius, height, color);
	drawrect(x1+width, y1, radius, height, color);
}

