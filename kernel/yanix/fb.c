#include <kernel/fb.h>
#include <drivers/video/video.h>
#include <fs/vfs_node.h>
#include <sys/types.h>
#include <yanix/sys/ioctl.h>
#include <libk/string.h>

int fb_cmd(int request, char *args)
{
	switch (request)
	{
	case FBIOGET_FSCREENINFO:
	case FBIOGET_VSCREENINFO:
		return video_get_screeninfo((struct fb_screeninfo *) args);
	}

	return -1;
}

ssize_t fb_write(vfs_node_t *node, unsigned int off, const void *buf,
                 size_t size, int flags)
{
	void *addr = (void *) (video_get_screen_fb() + off);
	memcpy(addr, buf, size);
	return size;
}