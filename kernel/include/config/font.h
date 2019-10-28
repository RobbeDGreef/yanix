#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONTHEIGHT 16
#define FONTWIDTH  10

uint8_t *font_getchar(char character);
uint8_t font_getchar_width(char character);
uint32_t font_getchar_index(char character);

#endif