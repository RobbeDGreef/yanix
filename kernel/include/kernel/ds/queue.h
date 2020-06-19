#ifndef _KERNEL_DS_QUEUE_H
#define _KERNEL_DS_QUEUE_H

#include <sys/types.h>

#define QUEUE_DEFAULT_SIZE 128

struct queue
{
	uintptr_t *buffer;
	size_t     size;
	size_t     capacity;
	int        lock;
};

struct queue *queue_create(int size);
void *        queue_pop(struct queue *q);
int           queue_push(struct queue *q, void *element);

#endif /* _KERNEL_DS_QUEUE_H */