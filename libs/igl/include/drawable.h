#ifndef _LIBGRAPHIC_DRAWABLE_H
#define _LIBGRAPHIC_DRAWABLE_H

struct window;

struct drawable
{
	struct window *window;
	int x;				/* The x location of the drawable */
	int y;				/* The y location of the drawable */
	float scale_x;
	float scale_y;
};

#endif /* _LIBGRAPHIC_DRAWABLE_H */