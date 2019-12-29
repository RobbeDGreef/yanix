#include <fs/vfs_node.h>
#include <stddef.h>

struct pipe
{
	void 			*buffer;
	unsigned int 	buffersize;
	unsigned int 	bufferused;
	
};

/**
 * @brief      Writes bytes to a pipe
 *
 * @param      node    The node
 * @param[in]  offset  The offset
 * @param[in]  buffer  The buffer
 * @param[in]  size    The size
 *
 * @return     On success, return the amount of bytes written. On failure, return -1 with errno set appropriately.
 */
ssize_t pipe_write(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);
	(void) (buffer);
	(void) (size);
	return 0;
}

/**
 * @brief      Reads bytes from a pipe
 *
 * @param      node    The node
 * @param[in]  offset  The offset
 * @param      buffer  The buffer
 * @param[in]  size    The size
 *
 * @return     On success, return the amount of bytes written. On failure, return -1 with errno set appropriately.
 */
ssize_t pipe_read(vfs_node_t *node, uint32_t offset, void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);
	(void) (buffer);
	(void) (size);
	return 0;
}

/**
 * @brief      Create an unnamed pipe in the system
 *
 * @param      pipefd  The pipefd ([0] is the read descriptor and [1] is the write descriptor)
 *
 * @return     On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 */
int pipe(int pipefd[2])
{
	(void) (pipefd);
	return 0;
}
