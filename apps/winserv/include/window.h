#ifndef _WS_WINDOW_H
#define _WS_WINDOW_H

#define YW_BPP 4
#define YW_DEF_X 100
#define YW_DEF_Y 100

struct ywindow
{
	char *title;
	
	int x;
	int y;

	int width;
	int height;

	void *buffer;
};

struct ywindow *yws_create_simple(char *title, int width, int height);
struct ywindow *yws_create(char *title, int x, int y, int width, int height);

#endif /* _WS_WINDOW_H */
