#ifndef VESATEXT_H
#define VESATEXT_H

#include <stdint.h>

typedef struct ttySystem_S {
	uint16_t	number;		// identification number
	char*		output;		// output to screen
	char*		input;		// input to screen
	uint16_t 	buffersize;
	uint32_t	col;
	uint32_t	row;
	uint32_t 	frontcolor;
	uint32_t	backgroundcolor;
} ttySystem;

void init_vesatext();
void vesa_print_char(char, uint32_t, uint32_t, uint32_t);
void vesa_print_at(char* message, int ,int, uint32_t);
void vesa_print(char* message);
void vesa_print_at_vgacolor(char* message, int col, int row, char color);
void vesa_clear_screen();
void init_vesa_cursor();
void vesa_lineclear();
void vesa_set_cursor(uint32_t col, uint32_t row);
uint32_t vesa_get_cursor_row();
uint32_t vesa_get_cursor_col();
#endif