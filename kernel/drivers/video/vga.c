/* @todo: Somehow sublime has removed the whole VGA system don't know why but it
 * means i gotta rewrite this */

#include <drivers/video/video.h>
#include <libk/string.h>

#define VGA_CARD_NAME   "vga_card"
#define VGA_DRIVER_NAME "yanix_vga_driver"

typedef struct vga_info_s
{
	unsigned int width;
	unsigned int height;
	unsigned int bpc;
	char *       fb;
} vga_info_t;

vga_info_t vga_info;

/**
 * @brief      Initializes the vga.
 *
 * @param[in]  width        The width
 * @param[in]  height       The height
 * @param[in]  bpc          The bpc
 * @param[in]  framebuffer  The framebuffer
 */
void init_vga(unsigned int framebuffer, unsigned int width, unsigned int height,
              unsigned int bpc)
{
	vga_info.width  = width;
	vga_info.height = height;
	vga_info.bpc    = bpc;
	vga_info.fb     = (char *) framebuffer;
}

/**
 * @brief      This function will turn the regular vga colors to well... vga
 * colors
 *
 * @param[in]  color  The color
 */
int vga_to_full(int color)
{
	return color;
}

/**
 * @brief      Draw a char at given offset
 *
 * @param[in]  character  The character
 * @param[in]  x          X location
 * @param[in]  y          Y location
 * @param[in]  frgcolor   The frgcolor
 * @param[in]  bgcolor    The bgcolor
 */
void vga_draw_char(char character, int x, int y, int frgcolor, int bgcolor)
{
	vga_info.fb[(y * vga_info.width + x) * vga_info.bpc] = character;
	vga_info.fb[(y * vga_info.width + x) * vga_info.bpc + 1] =
		(char) (bgcolor << 8) | (frgcolor & 0xF);
}

void vga_clear_screen()
{
	size_t chars = vga_info.width * vga_info.height;
	for (size_t i = 0; i < chars; i++)
	{
		/* @todo: VGA DRIVER: im not sure if the background has te be reset to 0
		 * or not */
		vga_info.fb[i * vga_info.bpc] = 0;
	}
}

/**
 * @brief      This function hooks the vga driver to the current video driver in
 * use
 *
 * @param      driver  The driver
 */
void hook_vga_to_video(video_driver_t *driver)
{
	/* First clear the driver settings */
	memset(driver, 0, sizeof(video_driver_t));

	/* Now hook all the functions */
	driver->draw_pixel = 0;
	driver->draw_char  = &vga_draw_char;
	driver->draw_rect  = 0;
	driver->draw_line  = 0;
	driver->clear      = &vga_clear_screen;

	driver->vga_to_full = &vga_to_full;

	/* And fill in all the extra data */
	driver->screen_width      = vga_info.width;
	driver->screen_height     = vga_info.height;
	driver->screen_bpp        = vga_info.bpc;
	driver->screen_fb         = (unsigned int) vga_info.fb;
	driver->video_driver_name = VGA_DRIVER_NAME;
	driver->video_card_name =
		VGA_CARD_NAME; /* This should actually be done by the PCI driver */
}
