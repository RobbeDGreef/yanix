#include <drivers/video/vesa.h>
#include <drivers/video/video.h>

#include <drivers/video/font.h>
#include <libk/bit.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <cpu/io.h>

#include <stddef.h>
#include <stdint.h>

#define VESA_DISP_INDEX 0x01ce
#define VESA_DISP_DATA  0x01cf

#define VESA_DISP_W      1
#define VESA_DISP_H      2
#define VESA_DISP_BPP    3
#define VESA_DISP_ENABLE 4
#define VESA_DISP_BANK   5
#define VESA_DISP_W_VIRT 6
#define VESA_DISP_H_VIRT 7

#define VESA_DISP_DISABLED    0
#define VESA_DISP_ENABLED     1
#define VESA_DISP_LFB_ENABLED 0x40

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

inline static void vesa_draw_pixel_inline(uint32_t *   videomem,
                                          unsigned int offset, uint32_t color)
{
	/*
	videomem[offset]     = color & 0xFF;
	videomem[offset + 1] = (color >> 8) & 0xFF;
	videomem[offset + 2] = (color >> 16) & 0xFF;
	*/

	videomem[offset] = color;
}

void set_reg(int port, int data)
{
	port_word_out(VESA_DISP_INDEX, port);
	port_word_out(VESA_DISP_DATA, data);
}

void init_vesa(void *physicallfb, unsigned int width, unsigned int height,
               unsigned int bpp)
{
	g_display = kmalloc(sizeof(vesa_ctrl_t));

	// Save all our variables in an handy struct
	g_display->physicalLFB = (void *) physicallfb;
	g_display->s_width     = width;
	g_display->s_height    = height;
	g_display->bpp         = bpp;

	// port_word_out(0x01CE, 3);
	// port_word_out(0x01CF, bpp * 8);

	/**
	 * @todo: we here asume there is a video mode available that supports these
	 *        we should check this at runtime and load the video modes from
	 *        real mode
	 */

	/* A lot of these are already set but we set them again to be sure */
	set_reg(VESA_DISP_ENABLE, VESA_DISP_DISABLED);
	set_reg(VESA_DISP_BPP, bpp * 8);
	set_reg(VESA_DISP_W, width);
	set_reg(VESA_DISP_H, height);
	set_reg(VESA_DISP_W_VIRT, width);
	set_reg(VESA_DISP_H_VIRT, height * 2);
	set_reg(VESA_DISP_ENABLE, VESA_DISP_ENABLED | VESA_DISP_LFB_ENABLED);
	set_reg(VESA_DISP_BANK, 0);
}

void vesa_draw_pixel_at_offset(unsigned int offset, int color)
{
	void *videomem = ((void *) (g_display->physicalLFB));
	vesa_draw_pixel_inline(videomem, offset, color);
}

void vesa_draw_pixel(int x, int y, int color)
{
	unsigned int location = (y * g_display->s_width + x) /** g_display->bpp*/;
	vesa_draw_pixel_at_offset(location, color);
}

int vesa_vga_to_full(int color)
{
	// This check prevents memory bugs
	if (color >= 0 && color <= 16)
	{
		return VGA_TO_FULL[color];
	}
	return 0;
}

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
	driver->video_card_name   = VESA_CARD_NAME; /* This should probably be
	                                             * done by the PCI driver */
}
