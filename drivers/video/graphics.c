#include <drivers/video/graphics.h>
#include <lib/math/math.h>
#include <lib/function.h>

#include <lib/bit/bit.h>
#include <lib/string/string.h>
#include <config/font.h>
#include <mm/paging.h>

#define True 1
#define False 0

unsigned char *videoMem;

unsigned char REFRESHLOOP;

extern page_directory_t *kernel_directory;


void vesa_switchToLFB(){
	videoMem = (unsigned char*)PhysicalLFB;
}



void refresh_display(){
	memcpy((void*)PhysicalLFB, videoMem, WIDTH*HEIGHT*(BPP/8));
	//for (uint32_t i = 0; i < WIDTH*HEIGHT*(BPP/8); i++){
	//	((uint8_t*)PhysicalLFB)[i] = videoMem[i];
	//}
}

void refresh_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height){
	uint32_t offset = y*WIDTH*(BPP/8) + x * (BPP/8);
	for (uint32_t i = 0; i < height; i++){
		for (uint32_t j = 0; j < width*(BPP/8); j++){
			((uint8_t*) PhysicalLFB)[offset+j] = videoMem[offset+j];
		}
		offset += (BPP/8)*WIDTH;
	}
}

void init_graphics_VESATEXT(){
	videoMem = (unsigned char*) PhysicalLFB;
}

void init_graphics(){
	// alocate Z-Buffer
	
	// allocated a part of memory as Z-Buffer; (0x240000 -> 2.25 mb)
	videoMem = (unsigned char*) ZBuffer;
	REFRESHLOOP = 1;
}

void quit_graphics(){
	REFRESHLOOP = 0;
}

void draw_at_offset(unsigned int offset, uint32_t color){
	videoMem[offset] = color & 255;
	videoMem[offset+1] = (color >> 8) & 255;
	videoMem[offset+2] = (color >> 16) & 255;
}

void draw_at_offset_safe(unsigned int offset, uint32_t color){
	if (offset < WIDTH*3*HEIGHT){
		videoMem[offset] = color & 255;
		videoMem[offset+1] = (color >> 8) & 255;
		videoMem[offset+2] = (color >> 16) & 255;
	}
}

void drawpixel(unsigned int x, unsigned int y, uint32_t color){
	if (x <= WIDTH && y <= HEIGHT){
		// in screen so we can draw
		//unsigned offset = x*3+y*(BPP*100);
		
		unsigned offset = y*WIDTH*3 + x*3;
		videoMem[offset] = color & 255;
		videoMem[offset+1] = (color >> 8) & 255;
		videoMem[offset+2] = (color >> 16) & 255;
	}
}

void drawrect(unsigned int topX, unsigned int topY, unsigned int width, unsigned int height, uint32_t color){
	unsigned offset = topY*WIDTH*(BPP/8) + topX*(BPP/8);
	for (unsigned int y = 0; y < height; y++){
		for (unsigned int x = 0; x < width; x++){
			offset += 3;
			videoMem[offset] = color & 255;
			videoMem[offset+1] = (color >> 8) & 255;
			videoMem[offset+2] = (color >> 16) & 255;
		}
		offset += WIDTH*(BPP/8) - width*(BPP/8);
	}
}

void drawrect_RAW(unsigned int topX, unsigned int topY, unsigned int width, unsigned int height, uint32_t color, uint8_t *memory){
	unsigned offset = topY*WIDTH*3;
	for (unsigned int y = topY; y < topY+height; y++){
		offset = topX*3+y*WIDTH*3;
		for (unsigned int x = topX; x < topX+width; x++){
			offset+=3;
			memory[offset] = color & 255;
			memory[offset+1] = (color >> 8) & 255;
			memory[offset+2] = (color >> 16) & 255;
		}
	}
}

void drawHline(unsigned int x, unsigned int y, unsigned int length, uint32_t color){
	unsigned offset = y*WIDTH*3+x*3;
	for (unsigned int i = 0; i < length; i++){
		offset += 3;
		draw_at_offset(offset, color);
	}
}

void drawHlineSafe(unsigned int x, unsigned int y, unsigned int length, uint32_t color){
	unsigned offset = y*WIDTH*3+x*3;
	for (unsigned int i = 0; i < length; i++){
		offset += 3;
		draw_at_offset_safe(offset, color);
	}
}

void drawVline(unsigned int x, unsigned int y, unsigned int length, uint32_t color){
	unsigned offset = x*3+y*WIDTH*3;
	for (unsigned int i = 0; i < length; i++){
		offset += WIDTH*3;
		draw_at_offset(offset, color);
	}
}

void drawVlineSafe(unsigned int x, unsigned int y, unsigned int length, uint32_t color){
	unsigned offset = x*3+y*WIDTH*3;
	for (unsigned int i = 0; i < length; i++){
		offset += WIDTH*3;
		draw_at_offset_safe(offset, color);
	}
}

// deprictated we are going to use bresenhams line drawing algorithm
void drawline(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color){
	if (x1 == x2){
		// horizontal line
		drawVline(x1, y1, (y2-y1), color);
	} else if (y1 == y2){
		// vertical line
		drawHline(x1, y1, (x2-x1), color);
	} else {
		// other line
		draw_line_bresenham(x1,y1,x2,y2,color);
	}
}

// bresenham
/*
pseudo:
 function line(x0, y0, x1, y1)
     real deltax := x1 - x0
     real deltay := y1 - y0
     real deltaerr := abs(deltay / deltax)    // Assume deltax != 0 (line is not vertical),
           // note that this division needs to be done in a way that preserves the fractional part
     real error := 0.0 // No error at start
     int y := y0
     for x from x0 to x1 
         plot(x,y)
         error := error + deltaerr
         if error ≥ 0.5 then
             y := y + sign(deltay) * 1
             error := error - 1.0

plotLine(x0,y0, x1,y1)
  dx = x1 - x0
  dy = y1 - y0
  D = 2*dy - dx
  y = y0

  for x from x0 to x1
    plot(x,y)
    if D > 0
       y = y + 1
       D = D - 2*dx
    end if
    D = D + 2*dy


*/

void draw_line_bresenham(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color){
	int dx = x2-x1;
	int dy = y2-y1;
	int d  = 2*dy-dx;
	int y  = y1;
	int xlen = x1+x2;
	for (int x = x1; x < xlen; x++){
		drawpixel(x,y,color);
		if (d > 0){
			y++;
			d -= 2*dx;
		}
		d += 2*dy;
	}
}

/*
pseudo:
	m = sqrt
*/

void draw_line_AA(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color){
	UNUSED(x1);
	UNUSED(x2);
	UNUSED(y2);
	UNUSED(y1);
	UNUSED(color);
}

static void _draw_circle(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    // left top
    drawpixel(x1-x, y1-y, color);
    drawpixel(x1-y, y1-x, color);
	
	// right top
    drawpixel(x1+x, y1-y, color);
    drawpixel(x1+y, y1-x, color);
    
    // left bottom
    drawpixel(x1-x, y1+y, color);
    drawpixel(x1-y, y1+x, color);
	
	// right bottom
	drawpixel(x1+x, y1+y, color);
    drawpixel(x1+y, y1+x, color);
    
}

static void _draw_circle_fill(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    // left top
    drawHlineSafe(x1-x, y1-y, x, color);
    drawHlineSafe(x1-y, y1-x, y, color);

	// right top
    drawHlineSafe(x1, y1-y, x, color);
    drawHlineSafe(x1, y1-x, y, color);

    // left bottom	
	drawHlineSafe(x1-x, y1+y, x, color);
	drawHlineSafe(x1-y, y1+x, y, color);

	// right bottom
    drawHlineSafe(x1, y1+y, x, color);
    drawHlineSafe(x1, y1+x, y, color);

}


void drawcircle(unsigned int x1, unsigned int y1, unsigned int radius, int fill, uint32_t color){
	int x = 0, y = radius;
    int d = 3 - 2 * radius;
   	if (fill == 1){
   		_draw_circle_fill(x1, y1, x, y, color);
   	}
   	else {
   		_draw_circle(x1, y1, x, y, color);
   	}

    while (y >= x) { 
        x++;
        if (d > 0){
            y--;
            d = d + 4 * (x - y) + 10; 
        }
        else {
            d = d + 4 * x + 6; 
        }
        if (fill == 1){
   			_draw_circle_fill(x1, y1, x, y, color);
   		}
   		else {
   			_draw_circle(x1, y1, x, y, color);
   		}
    }
}

static void drawcircle_left_top(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    drawpixel(x1-x, y1-y, color);
    drawpixel(x1-y, y1-x, color);
}

static void drawcircle_left_bot(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    drawpixel(x1-x, y1+y, color);
    drawpixel(x1-y, y1+x, color);
}

static void drawcircle_right_top(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
	drawpixel(x1+x, y1-y, color);
    drawpixel(x1+y, y1-x, color);
}

static void drawcircle_right_bot(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
	drawpixel(x1+x, y1+y, color);
    drawpixel(x1+y, y1+x, color);
}
static void drawcircle_left_top_fill(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    drawHlineSafe(x1-x, y1-y, x, color);
    drawHlineSafe(x1-y, y1-x, y, color);
}

static void drawcircle_right_top_fill(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
	// right top
    drawHlineSafe(x1, y1-y, x, color);
    drawHlineSafe(x1, y1-x, y, color);
}

static void drawcircle_left_bot_fill(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
    // left bottom
	drawHlineSafe(x1-x, y1+y, x, color);
	drawHlineSafe(x1-y, y1+x, y, color);
}

static void drawcircle_right_bot_fill(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, uint32_t color){
	// right bottom
    drawHlineSafe(x1, y1+y, x, color);
    drawHlineSafe(x1, y1+x, y, color);
}


void drawcirclequadrant(unsigned int x1, unsigned int y1, unsigned int radius, int fill, int quadrant, uint32_t color){
	int x = 0, y = radius;
    int d = 3 - 2 * radius;
   	
   	if (fill == True){
	   	if (quadrant == 1){
	   		drawcircle_right_top_fill(x1, y1, x, y, color);
	   	} else if (quadrant == 2){
	   		drawcircle_left_top_fill(x1, y1, x, y, color);
	   	} else if (quadrant == 3){
	   		drawcircle_left_bot_fill(x1, y1, x, y, color);
	   	} else if (quadrant == 4){
	   		drawcircle_right_bot_fill(x1, y1, x, y, color);
	   	}
    } else {
    	if (quadrant == 1){
	   		drawcircle_right_top(x1, y1, x, y, color);
	   	} else if (quadrant == 2){
	   		drawcircle_left_top(x1, y1, x, y, color);
	   	} else if (quadrant == 3){
	   		drawcircle_left_bot(x1, y1, x, y, color);
	   	} else if (quadrant == 4){
	   		drawcircle_right_bot(x1, y1, x, y, color);
	   	}
    }
    
    while (y >= x) { 
        x++;
        if (d > 0) 
        {
            y--;
            d = d + 4 * (x - y) + 10; 
        }
        else{
            d = d + 4 * x + 6; 
        }
	   	if (fill == True){
		   	if (quadrant == 1){
		   		drawcircle_right_top_fill(x1, y1, x, y, color);
		   	} else if (quadrant == 2){
		   		drawcircle_left_top_fill(x1, y1, x, y, color);
		   	} else if (quadrant == 3){
		   		drawcircle_left_bot_fill(x1, y1, x, y, color);
		   	} else if (quadrant == 4){
		   		drawcircle_right_bot_fill(x1, y1, x, y, color);
		   	}
	    } else {
	    	if (quadrant == 1){
		   		drawcircle_right_top(x1, y1, x, y, color);
		   	} else if (quadrant == 2){
		   		drawcircle_left_top(x1, y1, x, y, color);
		   	} else if (quadrant == 3){
		   		drawcircle_left_bot(x1, y1, x, y, color);
		   	} else if (quadrant == 4){
		   		drawcircle_right_bot(x1, y1, x, y, color);
		   	}
	    }  
    }
}

uint32_t colorIntensify(unsigned int color, unsigned char intensity){
	// intnsity is between 0-100
	// where 100 is full brightness
	if (intensity <= 100){
		intensity = 100/intensity;
		unsigned int R = (color >> 16);
		unsigned int G = (color << 16) >> 24;
		unsigned int B = (color << 24)>>24;
		unsigned int newcolor = R/intensity;
		newcolor = newcolor << 8;
		newcolor += G/intensity;
		newcolor = newcolor << 8;
		newcolor += B/intensity;
		return newcolor;
	} else {
		return color;
	}
}

void _draw_circle_AA(unsigned int x1, unsigned int y1, unsigned int x, unsigned int y, unsigned int radius, unsigned int color){
	    // left top
    float distance = (float) sqrt(y*y+x*x);
    float d = ((float) radius)-distance;
    d = absf(d);

    if (d != 0){
    	drawpixel(x1-y, y1-x, colorIntensify(color, 100/(d)));
    	drawpixel(x1-x, y1-y, colorIntensify(color, 100/(d)));
    } else {
    	drawpixel(x1-y, y1-x, color);
    	drawpixel(x1-x, y1-y, color);
    
    }
}

void drawcircleAA(unsigned int x1, unsigned int y1, unsigned int radius, int fill, uint32_t color){
	int x = 0, y = radius;
    int d = 3 - 2 * radius;
   	if (fill == 1){
   		_draw_circle_fill(x1, y1, x, y, color);
   	}
   	else {
   		_draw_circle_AA(x1, y1, x, y, radius, color);
   	}

    while (y >= x) { 
        x++;
        if (d > 0){
            y--;
            d = d + 4 * (x - y) + 10; 
        }
        else {
            d = d + 4 * x + 6; 
        }
        if (fill == 1){
   			_draw_circle_fill(x1, y1, x, y, color);
   		}
   		else {
   			_draw_circle_AA(x1, y1, x, y, radius, color);
   		}
    }
}



void drawchar(char character, uint32_t x, uint32_t y, uint32_t color){
	uint8_t *charpointer = getchar(character);
	
	//uint8_t width = getchar_width(character);
	//uint8_t padding = (FONTWIDTH - width) / 2;

	for (int l = 0; l < FONTHEIGHT; l++){
		for (int b = 0; b < FONTWIDTH; b++){
			//if (charpointer[l] == 0x00){
			if (getbit(charpointer[l], b)){
				drawpixel(x+(FONTWIDTH-b), y+l, color);
			}
		}
	}
}

void drawtext(char* message, uint32_t x, uint32_t y, uint32_t color){
	uint32_t i = 0;
	char c = message[i];
	while (c!=0){
		drawchar(c, x, y, color);
		x += FONTWIDTH;
		c = message[++i];

	}
} 

void clear_graphics(){
	memset((uint32_t*) PhysicalLFB, 0, WIDTH*3*HEIGHT);

	uint32_t *addr = (uint32_t*) PhysicalLFB; 
	for (uint32_t i = 0; i < MAXBUFFER/32; i++){
		addr[i] = 0x00000000;
	}
	
}
