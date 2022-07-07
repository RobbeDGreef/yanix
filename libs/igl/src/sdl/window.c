#include <idpl/sdl.h>
#include <SDL2_gfxPrimitives.h>
#include <window.h>
#include <core.h>
#include <events.h>

int g_color;

int sdl_create_window(struct window *win)
{
	SDL_Init(SDL_INIT_VIDEO);
	win->idpl.win = SDL_CreateWindow(win->title, win->x, win->y, win->width,
	                                 win->height, SDL_WINDOW_SHOWN);

	win->idpl.screen = SDL_GetWindowSurface(win->idpl.win);
	win->idpl.renderer =
		SDL_CreateRenderer(win->idpl.win, -1, SDL_RENDERER_ACCELERATED);

	sdl_drawcolor_set(win, win->foreground);
}

void sdl_quit()
{
	SDL_Quit();
	exit(0);
}

int sdl_update_window(struct window *win)
{
	sdl_pump_events(win);
	SDL_RenderPresent(win->idpl.renderer);
	SDL_Delay(20);
}

int sdl_drawcolor_set(struct window *win, int color)
{
	g_color = color << 8 | 0xFF;

	int blue  = color & 0xFF;
	int green = (color >> 8) & 0xFF;
	int red   = (color >> 16) & 0xFF;
	int alpha = (color >> 24) & 0xFF;
	alpha     = 0xFF;

	SDL_SetRenderDrawColor(win->idpl.renderer, red, green, blue, alpha);
}

void sdl_pump_events(struct window *win)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			sdl_quit();
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			{
				int press = (event.button.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
				int x     = event.button.x;
				int y     = event.button.y;
				_push_mouse_event(x, y, press, event.button.button);
			}
			break;
		}
	}
}

int sdl_draw_line(struct window *win, int x1, int y1, int x2, int y2, int t)
{
}

int sdl_draw_rect(struct window *win, int x, int y, int width, int height,
                  int fill)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = width;
	rect.h = height;

	if (fill)
		SDL_RenderFillRect(win->idpl.renderer, &rect);
	else
		SDL_RenderDrawRect(win->idpl.renderer, &rect);

	return 0;
}

void sdl_draw_circle(struct window *win, int x, int y, int radius, int fill)
{
	if (fill)
		circleColor(win->idpl.renderer, x, y, radius, g_color);
}

void sdl_draw_poly(struct window *win, struct coord *points, int amount,
                   int fill)
{
}

void sdl_draw_arc(struct window *win, int x, int y, int radius, int start,
                  int end, int fill)
{
	// if (fill)
	arcColor(win->idpl.renderer, x, y, radius, start, end, g_color);
}

void sdl_select_events(struct window *win, int request)
{
}

void sdl_clear_display(struct window *win)
{
	SDL_RenderClear(win->idpl.renderer);
}