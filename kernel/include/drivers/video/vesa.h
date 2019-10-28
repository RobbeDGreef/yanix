#ifndef _VESA_H
#define _VESA_H

#include <drivers/video/video.h>

typedef struct vesa_ctrl_s {
	void		 	*physicalLFB;	// location of the physical linear frame buffer (video memory)
	unsigned int 	s_width;		// screen width (in pixels)
	unsigned int 	s_height;		// screen height (in pixels)
	unsigned int 	bpp;			// bytes per pixel
} vesa_ctrl_t;


/**
 * @brief      This function hooks the vesa driver to the current video driver in use
 *
 * @param      driver  The driver
 */
void hook_vesa_to_video(video_driver_t *driver);

/**
 * @brief      Initializes all the data that vesa needs to startup
 *
 * @param      physicallfb  The physicallfb
 * @param[in]  width        The width
 * @param[in]  height       The height
 * @param[in]  bpp          The bits per pixel
 */
void init_vesa(void *physicallfb, unsigned int width, unsigned int height, unsigned int bpp);


#endif
