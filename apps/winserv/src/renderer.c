#include <window.h>
#include <eclib/vector.h>
#include <core.h>
#include <fcntl.h>
#include <unistd.h>
#include <yanix/fb.h>
#include <sys/ioctl.h>

struct screen
{
	int width;
	int height;

	void *zbuf;
	void *framebuf;
};

define_vector(struct ywindow *, winvec);

winvec g_winlist = NULL;
struct screen g_screen;

int renderer_pushwin(struct ywindow *win)
{
	return winvec_push(g_winlist, win);
}

static void copy_windowbuf(struct ywindow *win)
{
	int startpos = win->y * (g_screen.width * 4) + win->x * 4;
	char *loc = g_screen.zbuf + startpos;
	char *src = win->buffer;

	for (int i = 0; i < win->height; i++)
	{
		memcpy(loc, src, win->width*4);
		
		loc += g_screen.width * 4;
		src += win->width * 4;
	}
}

static void screenflip()
{
	/* We are assuming a 4 bytes per pixel framebuffer layout here */
	memcpy(g_screen.framebuf, g_screen.zbuf, g_screen.width * g_screen.height * 4);
}

int renderer_render()
{
	memset(g_screen.zbuf, 255, g_screen.width * g_screen.height * 4);
	int i;
	while ((i = winvec_iter(g_winlist)) != -1)
	{
		copy_windowbuf(winvec_get(g_winlist, i));
	}

	screenflip();
}

int renderer_init()
{
	g_winlist = winvec_create();

	/* Disable the terminal */
	int tty = open("/dev/tty", O_RDWR);
	if (tty == -1)
		return -1;

	ioctl(tty, 1);
	close(tty);

	int fb = open("/dev/fb0", O_RDWR);
	if (fb == -1)
		return -1;

	struct fb_screeninfo info;
	ioctl(fb, 1, &info);

	close(fb);

	g_screen.width = info.xres;
	g_screen.height = info.yres;
	g_screen.framebuf = info.framebuffer;

	/* We always create a 4 bytes per pixel screenlayout */
	g_screen.zbuf = malloc(g_screen.width * g_screen.height * 4);
	return 0;
}
