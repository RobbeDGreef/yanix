#ifndef VGA_H
#define VGA_H

#include <drivers/video/video.h>

/**
 * @brief      This function hooks the vga driver to the current video driver in use
 *
 * @param      driver  The driver
 */
void hook_vga_to_video(video_driver_t *driver);

/**
 * @brief      Initializes the vga.
 *
 * @param[in]  width        The width
 * @param[in]  height       The height
 * @param[in]  bpc          The bpc
 * @param[in]  framebuffer  The framebuffer
 */
void init_vga(unsigned int framebuffer, unsigned int width, unsigned int height, unsigned int bpc);

#endif