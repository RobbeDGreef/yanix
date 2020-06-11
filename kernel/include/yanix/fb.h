#ifndef _YANIX_FB_H
#define _YANIX_FB_H

#include <fs/vfs_node.h>
#include <sys/types.h>

struct fb_screeninfo
{
	unsigned long xres;
	unsigned long yres;
	unsigned long bits_per_pixel;
	void *        framebuffer;
};

int     fb_cmd(int request, char *args);
ssize_t fb_write(vfs_node_t *node, unsigned int off, const void *buf,
                 size_t size);

#endif /* _YANIX_FB_H */