#include <lgfx.h>
#include <core.h>

struct __gfx
{
	void *fb;
	int   bpp;
	int   w;
	int   h;
	int   fb_fd;
	void *dbuf;
	int   dbuf_size;
	int   dbuf_line;
};

struct __gfx *g_gfx = NULL;

int lgfx_init()
{
	int tty = open("/dev/tty", O_RDWR);
	if (tty == -1)
		return -1;

	ioctl(tty, 1);
	close(tty);

	int fb = open("/dev/fb0", O_RDWR);
	if (fb == -1)
		return -1;

	struct fb_screeninfo finfo;
	ioctl(fb, 1, &finfo);

	g_gfx = malloc(sizeof(struct __gfx));

	g_gfx->fb    = finfo.framebuffer;
	g_gfx->bpp   = finfo.bits_per_pixel;
	g_gfx->w     = finfo.xres;
	g_gfx->h     = finfo.yres;
	g_gfx->fb_fd = fb;
	printf("bpp: %i\n", g_gfx->bpp);

	g_gfx->dbuf_line = g_gfx->bpp * g_gfx->w;
	g_gfx->dbuf_size = g_gfx->bpp * g_gfx->w * g_gfx->h;
	g_gfx->dbuf      = malloc(g_gfx->dbuf_size);
	MEMSET(g_gfx->dbuf, 0, g_gfx->dbuf_size);

	return 0;
}

int lgfx_quit()
{
	int tty = open("/dev/tty", O_RDWR);
	if (tty == -1)
		return -1;

	ioctl(tty, 2);
	close(tty);

	close(g_gfx->fb_fd);
	free(g_gfx);
	return 0;
}

inline void lgfx_draw_point_raw(unsigned long offset, int color)
{
	int   l   = offset * g_gfx->bpp;
	char *buf = g_gfx->dbuf;

	buf[l]     = (color >> 16) & 0xFF;
	buf[l + 1] = (color >> 8) & 0xFF;
	buf[l + 2] = (color) &0xFF;
}

int lgfx_flip_display()
{
	MEMSET(g_gfx->dbuf, /* g_gfx->bg */ 0, g_gfx->dbuf_size);
	return 0;
}

int lgfx_update_display()
{
	MEMCPY(g_gfx->fb, g_gfx->dbuf, g_gfx->dbuf_size);
	return 0;
}

int lgfx_draw_frame(unsigned long frameptr)
{
	MEMCPY(g_gfx->fb, (void *) frameptr, g_gfx->dbuf_size);
	return 0;
}

void lgfx_draw_frame_32(unsigned long fp, int xres, int yres)
{
	char *    offset = g_gfx->fb;
	int       fbline = g_gfx->dbuf_line;
	uint32_t *frame  = (uint32_t *) fp;

	int linelen = xres * 4;

	int intptr = 0;
	for (int y = 0; y < yres; y++)
	{
		MEMCPY(offset, fp, linelen);
		fp += linelen;
		offset += fbline;

		/*

		for (int i = 0; i < linelen; i += 3)
		{
		    uint32_t pixel = frame[intptr++];
		    offset[i + 0]  = pixel & 0xFF;
		    offset[i + 1]  = (pixel >> 8) & 0xFF;
		    offset[i + 2]  = (pixel >> 16) & 0xFF;
		}
		*/
	}
}
