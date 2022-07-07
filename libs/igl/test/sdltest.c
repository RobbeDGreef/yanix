#include <SDL.h>

int main()
{
	SDL_Window * window;
	SDL_Surface *screen;

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Hello world", 0, 0, 1200, 600, SDL_WINDOW_SHOWN);
	screen = SDL_GetWindowSurface(window);

	/*
	while (1)
	{
	    SDL_FillRect(screen, NULL,
	                 SDL_MapRGB(screen->format, 0x33, 0x33, 0x33));
	    SDL_UpdateWindowSurface(window);
	}
	*/

	SDL_Delay(5000);

	SDL_DestroyWindow(window);

	SDL_Quit();
}