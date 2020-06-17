#ifndef _YANIX_YANIX_FB_H
#define _YANIX_YANIX_FB_H

struct fb_screeninfo
{
	unsigned long xres;
	unsigned long yres;
	unsigned long bits_per_pixel;
	void *        framebuffer;
};

#endif /* _YANIX_YANIX_FB_H */