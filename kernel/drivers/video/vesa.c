#include <drivers/video/vesa.h>
#include <drivers/video/video.h>

#include <config/font.h>
#include <libk/bit.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>

#include <stddef.h>
#include <stdint.h>

extern page_directory_t *g_kernel_directory;

/**
 * This array is just a way to convert the known vga terminal colors to full
 * vesa 16mil colors
 */
static const uint32_t VGA_TO_FULL[] = {0x000000, 0x0000aa, 0x00aa00, 0x00aaaa,
                                       0xaa0000, 0xaa00aa, 0xaa5500, 0xaaaaaa,
                                       0x555555, 0x5555ff, 0x55ff55, 0x55ffff,
                                       0xff5555, 0xff55ff, 0xffff55, 0xffffff};

/**
 * Just a printable name for the driver and card
 */
char *VESA_DRIVER_NAME = "yanix_vesa_driver";
char *VESA_CARD_NAME   = "vesa_card";

vesa_ctrl_t *g_display; // @todo: multiple monitor support

#include <debug.h>
#include <mm/paging.h>
/**
 * @brief      This is an inline function for drawing a pixel in videomemory
 *
 * @param      videomem  The videomem
 * @param[in]  offset    The offset
 * @param[in]  color     The color
 */
inline static void vesa_draw_pixel_inline(char *videomem, unsigned int offset,
                                          int color)
{
	videomem[offset]     = color & 0xFF;
	videomem[offset + 1] = (color >> 8) & 0xFF;
	videomem[offset + 2] = (color >> 16) & 0xFF;
}

/**
 * @brief      Initializes all the data that vesa needs to startup
 *
 * @param      physicallfb  The physicallfb
 * @param[in]  width        The width
 * @param[in]  height       The height
 * @param[in]  bpp          The bits per pixel
 */
void init_vesa(void *physicallfb, unsigned int width, unsigned int height,
               unsigned int bpp)
{
	g_display = kmalloc(sizeof(vesa_ctrl_t));
	// Save all our variables in an handy struct
	g_display->physicalLFB = (void *) physicallfb;
	g_display->s_width     = width;
	g_display->s_height    = height;
	g_display->bpp         = bpp;

	// unsigned int buffer_size = width * height * bpp;

	// allocate the VESA physical frame buffer bus
	// identity_map_memory_block((unsigned int) physicallfb, (unsigned
	// int)physicallfb + buffer_size, 0, 1, g_kernel_directory);
}

/**
 * @brief      Draws a pixel at offset without calculating offset
 *
 * @param[in]  offset  The offset
 * @param[in]  color   The color
 */
void vesa_draw_pixel_at_offset(unsigned int offset, int color)
{
	void *videomem = ((void *) (g_display->physicalLFB));
	vesa_draw_pixel_inline(videomem, offset, color);
}

/**
 * @brief      Draws a pixel at a location given by x and y
 *
 * @param[in]  x      The x axis location
 * @param[in]  y      The y axis location
 * @param[in]  color  The color
 */
void vesa_draw_pixel(int x, int y, int color)
{
	unsigned int location = (y * g_display->s_width + x) * g_display->bpp;
	vesa_draw_pixel_at_offset(location, color);
}

/**
 * @brief      Returns the appropriate rgb colorcode for a corresponding vga
 * color
 *
 * @param[in]  color  The color
 *
 * @return     The rgb color
 *
 * @note       This color has to have a value less than 16
 */
int vesa_vga_to_full(int color)
{
	// This check prevents memory bugs
	if (color >= 0 && color <= 16)
	{
		return VGA_TO_FULL[color];
	}
	return 0;
}

/**
 * @brief      Draws a character on desired location on screen
 *
 * @param[in]  character  The character
 * @param[in]  x          The x axis location
 * @param[in]  y          The y axis location
 * @param[in]  frgcolor   The frgcolor
 * @param[in]  bgcolor    The bgcolor
 *
 * @return     Success
 */
void vesa_draw_char(char character, int x, int y, int frgcolor, int bgcolor)
{
	/**
	 * @todo: This font/drawing system is realllyyy bad and we need to
	 * upgragetbitde it however for now it just works and it really is not of
	 * high priority
	 */
	unsigned char *cpointer = font_getchar(character);

	if (cpointer == 0)
	{
		return;
	}

	for (size_t l = 0; l < FONTHEIGHT; l++)
	{
		// looping over every line
		for (size_t b = 0; b < FONTWIDTH; b++)
		{
			// looping over every pixel
			int indent = (FONTWIDTH - font_getchar_width(character)) / 2;
			if (getbit((unsigned int) cpointer[l], b))
			{
				vesa_draw_pixel(x + (FONTWIDTH - b) + indent, y + l, frgcolor);
			}
			else
			{
				if (bgcolor != -1)
				{
					vesa_draw_pixel(x + (FONTWIDTH - b) + indent, y + l,
					                bgcolor);
				}
			}
		}
	}
}

void vesa_clear_screen_()
{
	memset(g_display->physicalLFB, 0,
	       g_display->bpp * g_display->s_width * g_display->s_height);
}

void vesa_clear_cell(int x, int y)
{
	int offset = ((y * FONTHEIGHT) * g_display->s_width + (x * FONTWIDTH))
				 * g_display->bpp;
	int line = g_display->s_width * g_display->bpp;
	for (int i = 0; i < FONTHEIGHT; i++)
	{
		// memset((char *) g_display->physicalLFB + offset, 0, FONTWIDTH *
		// g_display->bpp);
		offset += line;
	}
}

/**
 * @brief      This function hooks the vesa driver to the current video driver
 * in use
 *
 * @param      driver  The driver
 */
void hook_vesa_to_video(video_driver_t *driver)
{
	/* First clear the driver settings */
	memset(driver, 0, sizeof(video_driver_t));

	/* Now hook all the functions */
	driver->draw_pixel = &vesa_draw_pixel;
	driver->draw_char  = &vesa_draw_char;
	driver->draw_rect  = 0 /* &vesa_draw_rect */;
	driver->draw_line  = 0 /* &vesa_draw_line */;
	driver->clear      = &vesa_clear_screen_;
	driver->clear_cell = &vesa_clear_cell;

	driver->vga_to_full = &vesa_vga_to_full;

	/* And fill in all the extra data */
	driver->screen_width      = g_display->s_width;
	driver->screen_height     = g_display->s_height;
	driver->screen_bpp        = g_display->bpp;
	driver->screen_fb         = (unsigned int) g_display->physicalLFB;
	driver->video_driver_name = VESA_DRIVER_NAME;
	driver->video_card_name =
		VESA_CARD_NAME; /* This should actually be done by the PCI driver */
}
