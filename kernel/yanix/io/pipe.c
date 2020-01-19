// @todo: these files should be moved to the vfs instead of here in a weird io folder in the yanix kernel folder

#include <yanix/ds/circularbuffer.h>
#include <fs/filedescriptor.h>
#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <mm/heap.h>
#include <errno.h>
#include <yanix/pipe.h>

#include <debug.h>

/**
 * @brief      Closes a pipe
 *
 * @param      node  The vfs node of the pipe
 *
 * @return     Returns kfree. See kfree() for more info
 */
int pipe_close(vfs_node_t *node)
{
	kfree((void*) ((struct pipe_s*) node->offset)->circbuf);
	return kfree((void*) node->offset);
}

/**
 * @brief      Reads from a pipe
 *
 * @param      node    The vfs node of the pipe
 * @param[in]  offset  The pointer to the circular buffer structure
 * @param[in]  buffer  The buffer to write from
 * @param[in]  size    The amount of bytes to read from the pipe
 *
 * @return     On success, the amount of bytes read is returned. On failure, -1 is returned and errno is set appropriately.
 */
ssize_t pipe_read(vfs_node_t *node, uint32_t offset, void *buffer, size_t size)
{
	(void) (node);

	if (!offset)
	{
		errno = EPIPE;
		return -1;
	}
	
	struct pipe_s *pipe = (struct pipe_s*) offset;

	if (pipe->flags & NON_BLOCK)
		return 0;
	
	else
	{
		// waiting for a process to write to the pipe
		asm volatile("sti;");
		while (pipe->circbuf->virtual_begin == pipe->circbuf->virtual_end);
		asm volatile("cli;");
	}
	
	return circular_buffer_read((char*) buffer, size, pipe->circbuf);
}

/**
 * @brief      Writes to a pipe
 *
 * @param      node    The vfs node of the pipe
 * @param[in]  offset  The pointer to the circular buffer structure
 * @param[in]  buffer  The buffer to read from
 * @param[in]  size    The amount of bytes to write to the pipe
 *
 * @return     On success, the amount of bytes written is returned. On failure, -1 is returned and errno is set appropriately.
 */
ssize_t pipe_write(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);

	if (!offset)
	{
		errno = EPIPE;
		return -1;
	}

	// @todo: add blocking for when the pipe is full

	struct pipe_s *pipe = (struct pipe_s*) offset;

	return circular_buffer_write((char*) buffer, size, pipe->circbuf);
}

/**
 * @brief      Creates a pipe
 *
 * @details    Creates a pipe, a unidirectional data channel that can be used for interprocess communication. 
 *             These pipes will be destroyed when both ends of the channel are closed or the parent process which created the fd closes. 
 *
 * @param      pipefd  The pipe file descriptors, 0 is the read end and 1 is the write end.
 *
 * @return     On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 */
int pipe(int pipefd[2])
{
	struct pipe_s *pipe = kmalloc(sizeof(struct pipe_s));
	struct circular_buffer_s *circbuf = create_circular_buffer(0, CIRCULAR_BUFFER_OPTIMIZE_USHORTINT);

	vfs_node_t *pipe_node = vfs_setupnode("pipe", VFS_PIPE, 0, 0, 0, circbuf->size, (offset_t) pipe,
										  0, &pipe_close, 0, &pipe_read, &pipe_write, 0, 0, 0);

	if (!pipe_node)
		return -1;

	// @todo: the mode of this register file descriptor should be set 
	// @todo: separate read and write in these pipe file descriptors
	pipefd[0] = register_filedescriptor(pipe_node, 0);
	pipefd[1] = pipefd[0];

	pipe->pipefd[0] = pipefd[0];
	pipe->pipefd[1] = pipefd[1];
	pipe->circbuf = circbuf;

	return 0;
}
