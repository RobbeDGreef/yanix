#ifndef _IDPL_SDL_H
#define _IDPL_SDL_H

#include <SDL.h>

struct window;
struct coord;

struct sdl_idpl_wininfo
{
	SDL_Window *  win;
	SDL_Surface * screen;
	SDL_Renderer *renderer;
};

int  sdl_create_window(struct window *win);
int  sdl_update_window(struct window *win);
int  sdl_drawcolor_set(struct window *win, int color);
int  sdl_draw_line(struct window *win, int x1, int y1, int x2, int y2, int t);
int  sdl_draw_rect(struct window *win, int x1, int y1, int width, int height,
                   int fill);
void sdl_draw_circle(struct window *win, int x, int y, int radius, int fill);
void sdl_draw_poly(struct window *win, struct coord *points, int amount,
                   int fill);

void sdl_draw_arc(struct window *win, int x, int y, int radius, int start,
                  int end, int fill);

void sdl_select_events(struct window *win, int request);
void sdl_pump_events(struct window *win);
void sdl_quit();
void sdl_clear_display(struct window *win);

#endif /* _IDPL_SDL_H */