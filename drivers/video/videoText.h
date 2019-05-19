#ifndef VIDEOTEXT_H
#define VIDEOTEXT_H

//constants 
#define VIDEO_ADDR 0xb8000
#define MAX_ROW 25
#define MAX_COL 80
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#define WHITE_ON_BLACK 0x0f
#define GREEN_ON_BLACK 0x02
#define RED_ON_BLACK 0x04
#define BLUE_ON_BLACK 0x01

#define LGREEN_ON_BLACK 0x0a
#define LRED_ON_BLACK 0x0c
#define LBLUE_ON_BLACK 0x09

// api
void clear_screen();
void print_at(char *message, int col, int row);
void print(char *message);
void print_letter(char letter);

void lineClear(int n);
void print_color(char *message, char color);
void print_color_at(char *message, int col, int row, char color);
void print_int(int integer);
void print_binary_char(unsigned char binnum);
void print_hex(int hexnum);
void print_hex_full(int hexnum);
void print_hex_dump(void *ptr, unsigned int size);

void switchToLFB();

#endif