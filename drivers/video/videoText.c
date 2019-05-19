#include <drivers/video/videoText.h>
#include <drivers/video/vesaText.h>
#include <drivers/video/graphics.h>

#include <drivers/io/io.h>
#include <lib/string/string.h>
#include <lib/bit/bit.h>

#include <stdint.h>

extern int vesa_textmode;


// declarations

void set_cursor(int offset);
int get_cursor();
int get_screen_offset(int col, int row);
void print_char(char character, int col, int row, char color);
int handle_scrolling(int offset);

void lineClear(int n){
    if (vesa_textmode != 0) {
        vesa_lineclear(n);
        vesa_set_cursor(vesa_get_cursor_col()-n, vesa_get_cursor_row());
    } else {
        if (get_cursor() != 0){
            unsigned char *videomem = (unsigned char*) VIDEO_ADDR;
            for (int i = 0; i < n; i++){
                int offset = get_cursor();
                videomem[offset-2*(i+1)] = ' ';        
            }
            set_cursor(get_cursor()-2*(n));
        }
    }
}

void clear_screen(){
    if (vesa_textmode != 0){
        vesa_clear_screen();
    }
    else {
        const int screen_size = MAX_COL * MAX_ROW;
        uint8_t *screen = (uint8_t*) VIDEO_ADDR;
        for (int i = 0; i < screen_size; i++){
            screen[i*2] = ' ';
            screen[i*2+1] = WHITE_ON_BLACK;
        }
        set_cursor(get_screen_offset(0,0));
    }
}

void print_at(char *message, int col, int row){
    if (vesa_textmode != 0){
        vesa_print_at(message, col, row, 0xffffff);
    }
    else {
        if (col >= 0 && row >= 0){
            set_cursor(get_screen_offset(col, row));
        }

        for (unsigned int i = 0; message[i] != 0; i++){
            print_char(message[i], col, row, WHITE_ON_BLACK);
        }
    }
}

void print_color_at(char *message, int col, int row, char color){
    if (vesa_textmode != 0){
        vesa_print_at_vgacolor(message, col, row, color);
    } else {    
        if (col >= 0 && row >= 0){
            set_cursor(get_screen_offset(col, row));
        }
        for (unsigned int i = 0; message[i] != 0; i++){
            print_char(message[i], col, row, color);
        }
    }

}

void print(char *message){
    print_at(message, -1, -1);
}

void print_color(char *message, char color){
    print_color_at(message, -1, -1, color);
}


void print_letter(char letter){
    char str[2]; str[0] = letter; str[1] = 0;
    print_at(str, -1, -1);
}

void print_binary_char(unsigned char binnum) {
    for (size_t i = 8; i>0; i--){
        // loop over every bit
        if (getbit(binnum, i-1)){
            print("1");
        } else {
            print("0");
        }
    }
}

void print_hex(int hexnum){
    char str[16];
    hex_to_ascii(hexnum, str);
    print(str);
}

void print_hex_no_Ox(int hexnum) {
    char str[16];
    hex_to_ascii_no_Ox(hexnum, str);
    print(str);
}

#define ASCII_START 0x20 // (32)  start of printable chars ' '
#define ASCII_END   0x7F // (126) end   of printable chars '~'


void print_hex_dump(void *buf, unsigned int size) {
    // we will print 4 hex numbers after each other with a space in between 
    uint32_t *ptr = (uint32_t*) buf;
    uint32_t value = 0;
    uint8_t c = 0;
    uint8_t p = 0;
    char line[17]; // 16 chars + null byte
    memset(line, 0, 17);
    for (uint32_t i = 0; i < size; i += 0x10) {
        print_hex(((uint32_t) ptr)+i); print(": ");
        for (uint8_t j = 0; j < 4; j++){
            value = *(ptr + i/0x10*4 + j);
            //print("value: "); print_hex(value); print(" ");
            for (uint8_t x = 0; x < 4; x++){
                p = (uint8_t) (value >> x*8) & 0xFF;
                for (uint8_t y = 2; y > 0; y--) {
                    c = (p >> (y-1)*4) & 0xF;
                    //  >> (y-1)*8) & 0xF
                    //print("c: "); print_hex(c); print(" ");
                    if (c >= 0xA){
                        print_letter(c-0xA+'a');
                    } else {
                        print_letter(c+'0');
                    }
                }
                if (p >= ASCII_START && p <= ASCII_END){
                    line[j*4+x] = (char) p;
                } else {
                    line[j*4+x] = '_';
                }
                print(" ");
            }
            print("  ");
        }
        // now try to asciify it
        print("|");print(line); print("|");
        print("\n");
        memset(line, 0, 17);
    }
}

void print_int(int integer){
    char str[10];                                           // max length of int 32 bit (2 147 483 647)
    int_to_string(integer, str);
    print_at(str, -1, -1);
}

// private functions

int get_screen_offset(int col, int row){
    return (row*MAX_COL+col)*2;
}

int get_cursor() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset*2;
}

void set_cursor(int offset){
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char) (offset & 0xff));
}


void print_char(char character, int col, int row, char color){
	unsigned char *videomem = (unsigned char*)VIDEO_ADDR;
	if (!color){
		color = WHITE_ON_BLACK;
	}
	int offset;
	if (row >= 0 && col >= 0){
		offset = get_screen_offset(col, row);
	} else {
		offset = get_cursor();
	}

	if (character == '\n'){
		row = offset / (MAX_COL * 2);
		offset = get_screen_offset(79, row);
	} else{
		videomem[offset] = character;
		videomem[offset+1] = color;
	}
	offset += 2;
	offset = handle_scrolling(offset);
	set_cursor(offset);
}

int handle_scrolling(int cursor_offset){
    if (cursor_offset < MAX_ROW*MAX_COL*2){
        return cursor_offset;
    }

    //clear_screen();
    unsigned char *mem = (unsigned char *) VIDEO_ADDR;
    for (int i = 0; i<MAX_ROW; i++){
        for (int x = 0; x < MAX_COL*2; x++){
            mem[(i-1)*MAX_COL*2+x*2] = mem[i*MAX_COL*2+x*2];
            mem[(i-1)*MAX_COL*2+x*2+1] = mem[i*MAX_COL*2+x*2+1];     
        }
        //memcpy((uint32_t *)get_screen_offset(0, i) + VIDEO_ADDR, (uint32_t*) get_screen_offset(0, i-1) + VIDEO_ADDR, MAX_COL*2);
    }
    char* last_line = (char*) get_screen_offset(0, MAX_ROW - 1)+VIDEO_ADDR;
    for (int i = 0; i < MAX_COL*2; i++){
        last_line[i] = 0;
    }
    return cursor_offset - 2*MAX_COL;
    
}


void switchToLFB(){
    vesa_switchToLFB();
}