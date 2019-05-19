#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

#define PhysicalLFB 		0xfd000000
#define ZBuffer				0x1F400000


#define WIDTH				1024
#define HEIGHT				768
#define BPP					24
#define MAXBUFFER			WIDTH*HEIGHT*(BPP/8)

extern uint32_t LFB;

void init_graphics();

void draw_at_offset(unsigned int offset, uint32_t color);
void drawpixel(unsigned int x, unsigned int y, uint32_t color);

void drawline(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color);
void drawHline(unsigned int x, unsigned int y, unsigned int length, uint32_t color);
void drawVline(unsigned int x, unsigned int y, unsigned int length, uint32_t color);
void drawHlineSafe(unsigned int x, unsigned int y, unsigned int length, uint32_t color);
void drawVlineSafe(unsigned int x, unsigned int y, unsigned int length, uint32_t color);
void draw_line_bresenham(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color);
void draw_line_AA(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, uint32_t color);

void drawrect(unsigned int topX, unsigned int topY, unsigned int width, unsigned int height, uint32_t color);
void drawrect_RAW(unsigned int topX, unsigned int topY, unsigned int width, unsigned int height, uint32_t color, uint8_t *memory);


void drawcircle(unsigned int x1, unsigned int y1, unsigned int radius, int fill, uint32_t color);
void drawcirclequadrant(unsigned int x1, unsigned int y1, unsigned int radius, int fill, int quadrant, uint32_t color);

uint32_t colorIntensify(unsigned int color, unsigned char intensity);
void drawcircleAA(unsigned int x1, unsigned int y1, unsigned int radius, int fill, uint32_t color);
void drawchar(char character, uint32_t x, uint32_t y, uint32_t color);
void drawtext(char* message, uint32_t x, uint32_t y, uint32_t color);
void clear_graphics();

void init_graphics_VESATEXT();
void refresh_display();
void refresh_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

void vesa_switchToLFB();
#endif