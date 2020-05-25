#ifndef BIT_H
#define BIT_H

#define GETBIT(n, k) ((n >> k) & 0x1)

int getbit(int n, int k);

#endif