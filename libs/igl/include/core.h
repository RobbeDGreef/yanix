#ifndef _LIBGRAPHIC_CORE_H
#define _LIBGRAPHIC_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct coord
{
	short x;
	short y;
};

#include <switch.h>

struct drawable;

int           locTrans(struct drawable *draw, int x);
struct coord *coordTrans(struct drawable *draw, struct coord *points,
                         int amount);

#endif /* _LIBGRAPHIC_CORE_H */