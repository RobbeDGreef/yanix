#ifndef _LIBGRAPHIC_SWITCH_H
#define _LIBGRAPHIC_SWITCH_H

#ifdef IDPL_X11
	#include <idpl/x11.h>
	#define CREATE_WINDOW x11_create_window
	#define UPDATE_WINDOW x11_update_window
	#define DRAW_LINE     x11_draw_line
	#define DRAW_RECT     x11_draw_rect
	#define DRAW_CIRCLE   x11_draw_circle
	#define DRAW_POLYGON  x11_draw_poly
	#define DRAWCOLOR_SET x11_drawcolor_set
	#define DRAW_ARC      x11_draw_arc
	#define INIT_EVENTS   x11_select_events
	#define IDPL_WININFO  struct x11_idpl_wininfo
#endif

#ifdef IDPL_SDL
	#include <idpl/sdl.h>
	#define IDPL_WININFO  struct sdl_idpl_wininfo
	#define CREATE_WINDOW sdl_create_window
	#define UPDATE_WINDOW sdl_update_window
	#define DRAW_LINE     sdl_draw_line
	#define DRAW_RECT     sdl_draw_rect
	#define DRAW_CIRCLE   sdl_draw_circle
	#define DRAW_POLYGON  sdl_draw_poly
	#define DRAWCOLOR_SET sdl_drawcolor_set
	#define DRAW_ARC      sdl_draw_arc
	#define INIT_EVENTS   sdl_select_events
	#define CLEAR_DISPLAY sdl_clear_display
#endif

#endif /* _LIBGRAPHIC_SWITCH_H */