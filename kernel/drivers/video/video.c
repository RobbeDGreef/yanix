#include <stdint.h>
#include <drivers/video/vesa.h>
#include <drivers/video/vga.h>
#include <drivers/video/video.h>
#include <mm/heap.h>

unsigned int g_video_mode;

video_driver_t *g_video_driver;

/**
 * @brief      Sets the video mode.
 *
 * @param[in]  mode  The mode
 *
 * @return     success
 */
int set_video_mode(int mode)
{
	g_video_mode = mode;
	// @todo: This is a bad system, this system should find the appropriate driver dynamically instead of hardcoded like this 
	if (mode == VIDEO_MODE_TERM) {
		/* initialise vga driver */
		hook_vga_to_video(g_video_driver);	/* @todo: Sublime removed the VGA system so we gotta rewrite it because it somehow wasn't backed up :( */

	} else if (mode == VIDEO_MODE_VESA) {
		hook_vesa_to_video(g_video_driver);
	}
	return 0;
}

/**
 * @brief      Gets the video mode.
 *
 * @return     The video mode.
 */
int get_video_mode()
{
	return g_video_mode; 
}

/**
 * @brief      Initializes the video driver
 *
 * @param[in]  video_mode  The video mode
 *
 * @return     { description_of_the_return_value }
 */
int init_video(int video_mode)
{
	g_video_driver = kmalloc(sizeof(video_driver_t));
	set_video_mode(video_mode);
	return 0;
}

/**
 * @brief      Calculates a vga color to the colorset of the video driver
 *
 * @param[in]  vga_color  The vga color
 *
 * @return     The recalculated color
 */
int video_vga_to_rgb(int vga_color)
{
	return g_video_driver->vga_to_full(vga_color);
}

/**
 * @brief      Returns the screen width
 *
 * @return     Screen width
 */
int video_get_screen_width()
{
	return g_video_driver->screen_width;
}

/**
 * @brief      Returns the screen height
 *
 * @return     Screen height
 */
int video_get_screen_height()
{
	return g_video_driver->screen_height;
}

/**
 * @brief      Returns the amount of bytes per pixel of the screen
 *
 * @return     The bytes per pixel
 */
int video_get_screen_bpp()
{
	return g_video_driver->screen_bpp;
}

/**
 * @brief      Returns the address of the defined screen framebuffer
 *
 * @return     The screen frame buffer location
 */
int video_get_screen_fb()
{
	return g_video_driver->screen_fb;
}
/**
 * @brief      Draws a character at location
 *
 * @param[in]  c                 The character to draw
 * @param[in]  x                 x location of the draw
 * @param[in]  y                 y location of the draw
 * @param[in]  frontgroundcolor  The frontgroundcolor
 * @param[in]  backgroundcolor   The backgroundcolor
 */
void video_draw_char(char c, int x, int y, int frontgroundcolor, int backgroundcolor)
{
	if (g_video_driver->draw_char != 0) 
		g_video_driver->draw_char(c, x, y, frontgroundcolor, backgroundcolor);
}

void video_draw_rect(int x, int y, int width, int height, int color)
{
	if (g_video_driver->draw_rect != 0)
		g_video_driver->draw_rect(x, y, width, height, color);
}

void video_draw_line(int x1, int y1, int x2, int y2, int color)
{
	if (g_video_driver->draw_line != 0)
		g_video_driver->draw_line(x1, y1, x2, y2, color);
}

/**
 * @brief      Clears the screen
 */
void video_clear_screen()
{
	if (g_video_driver && g_video_driver->clear)
		g_video_driver->clear();
}

/**
 * @brief      Updates the cursor location on the screen
 *
 * @param[in]  x     New cursor x location 
 * @param[in]  y     New cursor y location
 */
void video_update_cursor(int x, int y)
{
	if (g_video_driver != 0 && g_video_driver->update_cursor != 0) {
		g_video_driver->update_cursor(x, y);
	}
}