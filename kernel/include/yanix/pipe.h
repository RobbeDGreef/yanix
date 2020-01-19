#ifndef PIPE_H
#define PIPE_H

#include <yanix/ds/circularbuffer.h>
#include <sys/types.h>

#define NON_BLOCK		(1 << 0)
#define BUFFERED		(1 << 1)


struct pipe_s
{
	struct circular_buffer_s 	*circbuf;
	int 						pipefd[2];
	flags_t 					flags;

};

int pipe_close(vfs_node_t *node);

ssize_t pipe_read(vfs_node_t *node, uint32_t offset, void *buffer, size_t size);
ssize_t pipe_write(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size);

int pipe(int pipefd[2]);

#endif /* PIPE_H */