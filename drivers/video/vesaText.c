#include <drivers/video/graphics.h>
#include <drivers/video/vesaText.h>
#include <config/font.h>
#include <lib/string/string.h>
#include <lib/function.h>
#include <proc/tasking.h>
#include <kernel/functions/kfunctions.h>

#include <stdint.h>

int vesa_textmode = 0;

uint32_t VESA_COLS;
uint32_t VESA_ROWS;

uint32_t conf_vesa_backgroundcolor;
uint32_t conf_vesa_frontcolor;



static const uint32_t VGA_TO_FULL[] = {
	0x000000,
	0x0000aa,
	0x00aa00,
	0x00aaaa,
	0xaa0000,
	0xaa00aa,
	0xaa5500,
	0xaaaaaa,
	0x555555,
	0x5555ff,
	0x55ff55,
	0x55ffff,
	0xff5555,
	0xff55ff,
	0xffff55,
	0xffffff
};

typedef struct cursor_struct{
	uint32_t col;
	uint32_t row;
} cursor;

cursor cursor_offset;

void init_vesatext(){
	init_graphics_VESATEXT();
	conf_vesa_frontcolor = 0xffffff;
	conf_vesa_backgroundcolor = 0x000000;
	vesa_textmode = 1;
	VESA_COLS = (uint32_t) WIDTH/FONTWIDTH;
	VESA_ROWS = (uint32_t) HEIGHT/FONTHEIGHT;
	cursor_offset.col = 0; cursor_offset.row = 0;
}

void vesa_print_char(char character, uint32_t col, uint32_t row, uint32_t color){
	drawchar(character, col*FONTWIDTH, row*FONTHEIGHT, color);
}

void memmove_2(uint8_t* source, uint8_t* dest, uint32_t bytes){
	for (uint32_t i = 0; i < bytes; i++){
        dest[i] = source[i];
    }
}

extern unsigned char *videoMem;

static int _check_scrolling()
{
	const int size = WIDTH*FONTHEIGHT;
	for (size_t i = 0; i < VESA_ROWS; i++) {
		memcpy(videoMem + i * size, videoMem + (i+1) * size, size);
	}
	return VESA_COLS-1;

}

void vesa_print_at(char* message, int col, int row, uint32_t color){
	if (col < 0 && row < 0){
		col = cursor_offset.col;
		row = cursor_offset.row;
	}
	if ((uint32_t) row >= VESA_ROWS-1){
		// todo: move everything up
		row = _check_scrolling();
	}
	
	//cursor_offset.col = col;
	//cursor_offset.row = row;

	char c = message[0];
	uint32_t i;
	for (i = 1; c != 0; i++){
		if ((uint32_t) row >= VESA_ROWS-1){
			row = _check_scrolling();
		}

		if (c == '\n'){
			col = 0;
			row++;

			cursor_offset.col = col;
			cursor_offset.row = row;
		} else {
			vesa_print_char(c, col, row, color);
			col++;
			if ((uint32_t) col == VESA_COLS){
				col = 0;
				row++;
				
			}

			cursor_offset.col = col;
			cursor_offset.row = row;
		}
		c = message[i];
	}
}

void vesa_print_at_vgacolor(char* message, int col, int row, char color){
	uint8_t front = ((color << 4) >> 4);
	uint32_t fullcolor = VGA_TO_FULL[front];
	vesa_print_at(message, col, row, fullcolor);
}

void vesa_print(char* message){
	vesa_print_at(message, -1, -1, conf_vesa_frontcolor);
}

void vesa_clear_screen(){
	clear_graphics();
	cursor_offset.col = 0;
	cursor_offset.row = 0;

}


void vesa_fill_block(int col, int row, uint32_t color){
	if (col < 0 && row < 0){
		col = cursor_offset.col;
		row = cursor_offset.row;
	}
	
	drawrect(col*FONTWIDTH, row*FONTHEIGHT, FONTWIDTH, FONTHEIGHT, color);
}

void vesa_lineclear(int n){
	uint32_t col = cursor_offset.col;
	uint32_t row = cursor_offset.row;

	for (; n != 0; n--){
		if (col == 0 && row != 0){
			row--;
			col = VESA_COLS-1;		
		} else if (col == 0 && row == 0){
			// don't do anything
		} else{
			col--;
		}
		vesa_fill_block(col, row, conf_vesa_backgroundcolor);
	}
}

void vesa_set_cursor(uint32_t col, uint32_t row) {
	if (col < VESA_COLS && row < VESA_ROWS){	
		cursor_offset.col = col;
		cursor_offset.row = row;
	}
}

uint32_t vesa_get_cursor_col(){
	return cursor_offset.col;
}

uint32_t vesa_get_cursor_row(){
	return cursor_offset.row;
}

#if 0

static void cursorblink(){ 	// todo: this is super inefficient (create a better sleep system that does stuff while task sleeps)
	return;
	while (1){
		vesa_fill_block(-1,-1,conf_vesa_backgroundcolor);
		sleep(200);
		vesa_print_char('_', cursor_offset.col,cursor_offset.row, conf_vesa_frontcolor);
		sleep(200);
	}
}

void init_vesa_cursor(){
	/*
	this function will initialize the cursor blinking
	by making use of tasking and creating a task solely for this purpose
	*/
	//fork_func(cursorblink);
}

#endif