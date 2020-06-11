#ifndef _LIBGFX_LGFX_H
#define _LIBGFX_LGFX_H

int  lgfx_init();
int  lgfx_quit();
int  lgfx_flip_display();
void lgfx_draw_point_raw(unsigned long offset, int color);
int  lgfx_update_display();
int  lgfx_draw_frame(unsigned long fp);
void lgfx_draw_frame_32(unsigned long fp, int xres, int yres);

#endif /* _LIBGFX_LGFX_H */