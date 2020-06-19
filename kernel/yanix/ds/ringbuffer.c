#include <libk/string.h>
#include <mm/heap.h>
#include <stddef.h>
#include <kernel/ds/ringbuffer.h>

#include <const.h>
#include <errno.h>

/**
 * @brief      Creates a circular buffer.
 *
 * @param[in]  size   The size
 * @param[in]  flags  The flags
 *
 * @return     Reference to the buffer.
 */
struct ringbuffer *create_ringbuffer(size_t size, cb_flags_t flags)
{
	struct ringbuffer *circbuf = kmalloc(sizeof(struct ringbuffer));

	memset(circbuf, 0, sizeof(struct ringbuffer));

	if (flags & RINGBUFFER_OPTIMIZE_USHORTINT)
		size = USHRT_MAX;

	circbuf->buffer_start = kmalloc(size);
	circbuf->buffer_end   = circbuf->buffer_start + size;
	circbuf->flags        = flags;
	circbuf->size         = size;

	circbuf->virtual_begin = 0;
	circbuf->virtual_end   = 0;

	return circbuf;
}

void ringbuffer_destroy(struct ringbuffer *rb)
{
	kfree(rb->buffer_start);
	kfree(rb);
}

#include <debug.h>

/**
 * @brief      Reads from a circular buffer beginning from a given index.
 *
 * @param      buffer   The buffer to write to
 * @param[in]  size     The amount of bytes to read
 * @param[in]  index    The index to start reading at
 * @param      circbuf  The circular buffer structure to read from
 *
 * @return     On success, the amount of bytes written to the buffer. On
 * failure, -1 is returned and errno is set appropriately.
 */
ssize_t ringbuffer_read_index(char *buffer, size_t size, unsigned long index,
                              struct ringbuffer *circbuf)
{
	if (circbuf->lock && circbuf->lock < size)
		size = circbuf->lock;

	if (circbuf->flags & RINGBUFFER_OPTIMIZE_USHORTINT)
	{
		/**
		 * This is an optimized version of the regular circular buffer
		 * implementation this implmementation uses a unsigned short int as a
		 * way to automatically wrap around the buffer size when the buffer
		 * exceeds the 64k max. It is kind of a hack yes but it is very very
		 * efficient.
		 */
		unsigned short int ind = index;
		size_t             max = size;

		if (max > circbuf->virtual_end - circbuf->virtual_begin)
			max = circbuf->virtual_end - circbuf->virtual_begin;

		for (size_t i = 0; i < max; i++)
		{
			buffer[i] = circbuf->buffer_start[ind++];
		}

		/**
		 * @Warning: this implementation will reset the vritual begin every time
		 * you read
		 */
		circbuf->virtual_begin = index + max;

		if (circbuf->lock)
			circbuf->lock -= max;

		return max;
	}
	else
	{
		// @todo: other implmementations should be supported aswell
		errno = ENOSYS;
		return -1;
	}
}

/**
 * @brief      Reads from the start location of a circular buffer
 *
 * @param      buffer   The buffer to write to
 * @param[in]  size     The amount of bytes read
 * @param      circbuf  The circular buffer structure to read from
 *
 * @return     On success, the amount of bytes read from the circular buffer. On
 * failure, -1 is returned and errno is set appropriately.
 */
ssize_t ringbuffer_read(char *buffer, size_t size, struct ringbuffer *circbuf)
{
	return ringbuffer_read_index(buffer, size, circbuf->virtual_begin, circbuf);
}

/**
 * @brief      Writes from a circular buffer beginning from a given index.
 *
 * @param      buffer   The buffer to write from
 * @param[in]  size     The amount of bytes to write
 * @param[in]  index    The index to start writing at
 * @param      circbuf  The circular buffer structure to write to
 *
 * @return     On success, the amount of bytes read from the buffer. On failure,
 * -1 is returned and errno is set appropriately.
 */
ssize_t ringbuffer_write_index(char *buffer, size_t size, unsigned long index,
                               struct ringbuffer *circbuf)
{
	if (circbuf->flags & RINGBUFFER_OPTIMIZE_USHORTINT)
	/**
	 * This is an optimized version of the regular circular buffer
	 * implementation this implmementation uses a unsigned short int as a way to
	 * automatically wrap around the buffer size when the buffer exceeds the 64k
	 * max. It is kind of a hack yes but it is very very efficient.
	 */
	{
		unsigned short int ind = index;

		for (size_t i = 0; i < size; i++)
		{
			if (buffer[i] == '\n')
			{
				circbuf->lock =
					circbuf->virtual_end - circbuf->virtual_begin + i + 1;
			}

			circbuf->buffer_start[ind++] = buffer[i];
		}

		circbuf->virtual_end = index + size;
		return size;
	}
	else
	{
		// @todo: other implmementations should be supported aswell
		errno = ENOSYS;
		return -1;
	}
}

/**
 * @brief      Writes to the start location of a circular buffer
 *
 * @param      buffer   The buffer to write from
 * @param[in]  size     The amount of bytes written
 * @param      circbuf  The circular buffer structure to write to
 *
 * @return     On success, the amount of bytes written to the circular buffer.
 * On failure, -1 is returned and errno is set appropriately.
 */
ssize_t ringbuffer_write(char *buffer, size_t size, struct ringbuffer *circbuf)
{
	return ringbuffer_write_index(buffer, size, circbuf->virtual_end, circbuf);
}

void ringbuffer_block(struct ringbuffer *circbuf)
{
	while (!circbuf->lock)
		;
}

void ringbuffer_flush(struct ringbuffer *circbuf)
{
	circbuf->virtual_begin = circbuf->virtual_end;
	circbuf->lock          = 0;
}

int ringbuffer_remove(int location, struct ringbuffer *circbuf)
{
	if (!(circbuf->virtual_end - circbuf->virtual_begin))
		return -1;

	if (circbuf->flags & RINGBUFFER_OPTIMIZE_USHORTINT)
	{
		uint16_t base = (uint16_t) circbuf->virtual_end - location - 1;
		/* shift the bytes */
		for (int i = 0; i < location - 1; i++)
		{
			circbuf->buffer_start[base + i] =
				circbuf->buffer_start[base + i + 1];
		}

		circbuf->buffer_start[circbuf->virtual_end] = '\0';
		circbuf->virtual_end--;
		return 0;
	}
	else
	{
		// @todo: other implmementations should be supported aswell
		errno = ENOSYS;
		return -1;
	}
}