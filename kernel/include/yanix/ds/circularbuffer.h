#ifndef CIRC_BUF
#define CIRC_BUF

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define CIRCULAR_BUFFER_OPTIMIZE_USHORTINT 	(1 << 0)

/// Define a circular buffer flags type
typedef uint32_t cb_flags_t;

struct circular_buffer_s
{
	unsigned long	size;

	unsigned long 	virtual_end;	// Will hold the current end of the list
	unsigned long 	virtual_begin;	// Will hold the current begin of the list 
									// (since it is circular the actual one will be abstracted away)

	char 			*buffer_start;
	char			*buffer_end;

	unsigned int	lock;

	cb_flags_t 		flags;
};

struct circular_buffer_s *create_circular_buffer(size_t size, cb_flags_t flags);

ssize_t circular_buffer_read_index(char *buffer, size_t size, unsigned long index, struct circular_buffer_s *circbuf);
ssize_t circular_buffer_read(char *buffer, size_t size, struct circular_buffer_s *circbuf);

ssize_t circular_buffer_write_index(char *buffer, size_t size, unsigned long index, struct circular_buffer_s *circbuf);
ssize_t circular_buffer_write(char *buffer, size_t size, struct circular_buffer_s *circbuf);
void circular_buffer_block(struct circular_buffer_s *circbuf);
void circbuf_buffer_flush(struct circular_buffer_s *circbuf);
int circular_buffer_remove(int location, struct circular_buffer_s *circbuf);

#endif // CIRC_BUF