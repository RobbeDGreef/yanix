#ifndef CENTRAL_VIDEO_H
#define CENTRAL_VIDEO_H

#define VIDEO_MODE_TERM 0
#define VIDEO_MODE_VESA 1

// Function pointers for drawing functions
typedef void (*draw_char_fpointer)(char c, int x, int y, int foregroundcolor,
                                   int backgroundcolor);
typedef void (*draw_pixel_fpointer)(int x, int y, int color);
typedef void (*draw_rect_fpointer)(int x, int y, int width, int height,
                                   int color);
typedef void (*draw_line_fpointer)(int x, int y, int x2, int y2, int color);
typedef void (*clear_fpointer)();
typedef void (*clear_cell_fpointer)(int x, int y);
typedef void (*updt_cursor_fpointer)(int x, int y);

// Extra function pointers
typedef int (*vga_to_full_fpointer)(int color);

/* This will hold the functions of the video driver */
typedef struct video_driver_s
{
	draw_pixel_fpointer  draw_pixel;
	draw_char_fpointer   draw_char;
	draw_rect_fpointer   draw_rect;
	draw_line_fpointer   draw_line;
	clear_fpointer       clear;
	clear_cell_fpointer  clear_cell;
	updt_cursor_fpointer update_cursor;

	vga_to_full_fpointer vga_to_full;

	unsigned int screen_width;
	unsigned int screen_height;
	unsigned int screen_bpp;
	unsigned int screen_fb;
	char *       video_driver_name;
	char *       video_card_name;
} video_driver_t;

/**
 * @brief      Sets the video mode.
 *
 * @param[in]  mode  The mode
 *
 * @return     success
 */
int set_video_mode(int mode);

/**
 * @brief      Gets the video mode.
 *
 * @return     The video mode.
 */
int get_video_mode();

/**
 * @brief      Initializes the video driver
 *
 * @param[in]  video_mode  The video mode
 *
 * @return     { description_of_the_return_value }
 */
int init_video(int video_mode);

/**
 * @brief      Calculates a vga color to the colorset of the video driver
 *
 * @param[in]  vga_color  The vga color
 *
 * @return     The recalculated color
 */
int video_vga_to_rgb(int vga_color);

/**
 * @brief      Returns the screen width
 *
 * @return     Screen width
 */
int video_get_screen_width();

/**
 * @brief      Returns the screen height
 *
 * @return     Screen height
 */
int video_get_screen_height();

/**
 * @brief      Returns the address of the defined screen framebuffer
 *
 * @return     The screen frame buffer location
 */
int video_get_screen_fb();

/**
 * @brief      Returns the amount of bytes per pixel of the screen
 *
 * @return     The bytes per pixel
 */
int video_get_screen_bpp();

/**
 * @brief      Draws a character at location
 *
 * @param[in]  c                 The character to draw
 * @param[in]  x                 x location of the draw
 * @param[in]  y                 y location of the draw
 * @param[in]  frontgroundcolor  The frontgroundcolor
 * @param[in]  backgroundcolor   The backgroundcolor
 */
void video_draw_char(char c, int x, int y, int frontgroundcolor,
                     int backgroundcolor);

/**
 * @brief      Updates the cursor location on the screen
 *
 * @param[in]  x     New cursor x location
 * @param[in]  y     New cursor y location
 */
void video_update_cursor(int x, int y);

/**
 * @brief      Clears the screen
 */
void video_clear_screen();

/**
 * @brief      Clears a single cell
 *
 * @param[in]  x     the x location of the cell
 * @param[in]  y     the y location of the cell
 */
void video_clear_cell(int x, int y);

#endif