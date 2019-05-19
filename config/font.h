#ifndef FONT_H
#define FONT_H

#include <stdint.h>

uint8_t* getchar(char character);
uint8_t getchar_width(char character);
uint32_t getchar_index(char character);
extern uint8_t FONTHEIGHT;
extern uint8_t FONTWIDTH;

#endif