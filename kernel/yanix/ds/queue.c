#include <kernel/ds/queue.h>
#include <mm/heap.h>
#include <libk/string.h>
#include <kernel/atomic.h>

/* Simple spinlock */
void queue_lock(struct queue *q)
{
	/* Atomic_test_and_set returns 1 on failure */
	while (q->lock && atomic_test_and_set(&q->lock, 1))
		;
}

void queue_unlock(struct queue *q)
{
	q->lock = 0;
}

struct queue *queue_create(int size)
{
	if (size == -1)
		size = QUEUE_DEFAULT_SIZE;

	struct queue *q = kmalloc(sizeof(struct queue));
	memset(q, 0, sizeof(struct queue));
	q->buffer   = kmalloc(size * sizeof(uintptr_t));
	q->size     = 0;
	q->capacity = size;

	return q;
}

void *queue_pop(struct queue *q)
{
	queue_lock(q);

	if (!q->size)
		return NULL;

	q->size--;
	void *element = (void *) *q->buffer;

	memcpy(q->buffer, &q->buffer[1], sizeof(uintptr_t) * q->size);

	queue_unlock(q);
	return element;
}

int queue_push(struct queue *q, void *element)
{
	queue_lock(q);
	if (q->capacity == q->size)
		return -1;

	q->buffer[q->size++] = (uintptr_t) element;
	int ret              = q->size - 1;
	queue_unlock(q);
	return ret;
}