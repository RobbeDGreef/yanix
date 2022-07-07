#ifndef __ECLIB_RINGBUF_H
#define __ECLIB_RINGBUF_H

#include <limits.h>

#define ECLIB_RINGBUF_OPTIMIZE_USHRTINT 1

#define define_ringbuf_type(name)
struct ringbuf_s_##name
{
	void * buffer;
	size_t bufsize;
	int    flags;
	int    head;
	int    tail;
};
typedef ringbuf_s_##name *ringbuf_##name;

#define define_ringbuf_create(type, name)
ringbuf_##name ringbuf_##name##_create(int size)
{
	ringbuf_##name rb = eclib_malloc(sizeof(struct ringbuf_s_##name));
	eclib_memset(rb, 0, sizeof(struct ringbuf_s_##name));

	rb->buffer = eclib_malloc(size);
	if (size == USHRT_MAX)
		flags = ECLIB_RINGBUF_OPTIMIZE_USHRTINT;
}

#define define_ringbuf_read(type, name)
type ringbuf_##name##_read(ringbuf_##name rb, void *buffer, size_t len)
{
	for (uint i = 0; i < len; i++)
	{
		if (rb->head)
			buffer[rb->head] = rb->buffer[rb->head];
		rb->head++;
	}
}

#define define_ringbuf_write(type, name)

#endif /* _ECLIB_RINGBUF_H */