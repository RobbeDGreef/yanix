#ifndef PIPE_H
#define PIPE_H

#include <sys/types.h>
#include <yanix/ds/circularbuffer.h>

#define NON_BLOCK (1 << 0)
#define BUFFERED  (1 << 1)

struct pipe_s
{
	struct circular_buffer_s *circbuf;
	int                       pipefd[2];
	flags_t                   flags;
};

int pipe_close(vfs_node_t *node);

ssize_t pipe_read(vfs_node_t *node, unsigned int offset, void *buffer,
                  size_t size);
ssize_t pipe_write(vfs_node_t *node, unsigned int offset, const void *buffer,
                   size_t size);
ssize_t pipe_remove(vfs_node_t *node, unsigned int offset, int location);

int            pipe(int pipefd[2]);
struct pipe_s *pipe_create();
int            mkfifo(const char *path);

#endif /* PIPE_H */