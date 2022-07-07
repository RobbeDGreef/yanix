#include <core.h>
#include <drawable.h>

int locTrans(struct drawable *draw, int x)
{
	return x + draw->x;
}

struct coord *coordTrans(struct drawable *draw, struct coord *points,
                         int amount)
{
	for (int i = 0; i < amount; i++)
	{
		points[i].x += draw->x;
		points[i].y += draw->y;
	}

	return points;
}