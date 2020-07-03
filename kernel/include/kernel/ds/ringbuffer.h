#ifndef _KERNEL_DS_RINGBUFFER_H
#define _KERNEL_DS_RINGBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <kernel/atomic.h>

#define RINGBUFFER_OPTIMIZE_USHORTINT (1 << 0)

/// Define a circular buffer flags type
typedef uint32_t cb_flags_t;

struct ringbuffer
{
	unsigned long size;

	volatile unsigned long virtual_end; // Will hold the current end of the list
	volatile unsigned long virtual_begin; // Will hold the current begin of the
	                                      // list (since it is circular the
	                                      // actual one will be abstracted away)

	char *buffer_start;
	char *buffer_end;

	volatile unsigned int lock;
	struct spinlock spinlock;

	cb_flags_t flags;
};

struct ringbuffer *create_ringbuffer(size_t size, cb_flags_t flags);
void               ringbuffer_destroy(struct ringbuffer *rb);

ssize_t ringbuffer_read_index(char *buffer, size_t size, unsigned long index,
                              struct ringbuffer *circbuf);
ssize_t ringbuffer_read(char *buffer, size_t size, struct ringbuffer *circbuf);

ssize_t ringbuffer_write_index(char *buffer, size_t size, unsigned long index,
                               struct ringbuffer *circbuf);
ssize_t ringbuffer_write(char *buffer, size_t size, struct ringbuffer *circbuf);
void    ringbuffer_block(struct ringbuffer *circbuf);
void    ringbuffer_flush(struct ringbuffer *circbuf);
int     ringbuffer_remove(int location, struct ringbuffer *circbuf);

#endif /* _KERNEL_DS_RINGBUFFER_H */