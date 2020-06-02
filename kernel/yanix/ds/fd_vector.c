/**
 * @defgroup   FD_VECTOR fd vector
 *
 * @brief      This file implements a c++ like vector for file descriptors.
 *
 * @author     Root
 * @date       2020
 */

#include <fs/filedescriptor.h>
#include <fs/vfs_node.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <yanix/ds/fd_vector.h>

vector_t *vector_create()
{
	vector_t *vec = kmalloc(sizeof(vector_t));

	if (!vec)
		return 0;

	memset(vec, 0, sizeof(vector_t));

	vec->vector_buffer =
		kmalloc(sizeof(struct file_descriptor) * VECTOR_BEGIN_AMOUNT);

	if (!vec->vector_buffer)
		return 0;

	memset(vec->vector_buffer, 0,
	       sizeof(struct file_descriptor) * VECTOR_BEGIN_AMOUNT);

	vec->vector_maxsize = VECTOR_BEGIN_AMOUNT;

	return vec;
}

vector_t *vector_copy(vector_t *cp)
{
	vector_t *vec = kmalloc(sizeof(vector_t));

	if (!vec)
		return 0;

	vec->vector_buffer =
		kmalloc(sizeof(struct file_descriptor) * cp->vector_maxsize);

	if (!vec->vector_buffer)
		return 0;

	vec->vector_maxsize = cp->vector_maxsize;
	vec->vector_size    = cp->vector_size;

	memcpy(vec->vector_buffer, cp->vector_buffer,
	       cp->vector_maxsize * sizeof(struct file_descriptor));

	return vec;
}

int vector_expand(vector_t *vec)
{
	int                     newsize = vec->vector_maxsize * 2;
	struct file_descriptor *tmp =
		kmalloc(sizeof(struct file_descriptor) * newsize);

	if (!tmp)
		return -1;

	memset(tmp, 0, newsize * sizeof(struct file_descriptor));
	memcpy(tmp, vec->vector_buffer,
	       vec->vector_maxsize * sizeof(struct file_descriptor));

	kfree(vec->vector_buffer);
	vec->vector_buffer  = tmp;
	vec->vector_maxsize = newsize;

	return 0;
}

int vector_push(vector_t *vec, struct file_descriptor fd)
{
	if (vector_expand(vec))
		return -1;

	memcpy(&vec->vector_buffer[vec->vector_size++], &fd,
	       sizeof(struct file_descriptor));

	return vec->vector_size;
}

int vector_add(vector_t *vec, struct file_descriptor fd)
{
	for (int i = 0; i < vec->vector_maxsize; i++)
	{
		if (vec->vector_buffer[i].node == 0)
		{
			vec->vector_size++;
			memcpy(&vec->vector_buffer[i], &fd, sizeof(struct file_descriptor));
			return i;
		}
	}

	return vector_push(vec, fd);
}

int vector_clear(vector_t *vec, int fd)
{
	vec->vector_buffer[fd].node = 0;
	vec->vector_size--;
	return 0;
}

struct file_descriptor *vector_get(vector_t *vec, int fd)
{
	if (vec && fd <= vec->vector_size)
		return &vec->vector_buffer[fd];

	return 0;
}

struct file_descriptor *vector_get_node(vector_t *vec, vfs_node_t *node)
{
	if (vec && node)
	{
		for (int i = 0; i < vec->vector_size; i++)
		{
			if (vec->vector_buffer[i].node == node)
				return &vec->vector_buffer[i];
		}
	}
	return 0;
}

int vector_destroy(vector_t *vec)
{
	kfree(vec->vector_buffer);
	kfree(vec);
	return 0;
}

int vector_add_from(vector_t *vec, struct file_descriptor fd, int from)
{
	if (from > vec->vector_maxsize)
		vector_expand(vec, roundup(from, vec->vector_maxsize));

	for (int i = from; i < vec->vector_maxsize; i++)
	{
		if (vec->vector_buffer[i].node == 0)
		{
			if (i >= vec->vector_size)
				vec->vector_size = i + 1;

			memcpy(&vec->vector_buffer[i], &fd, sizeof(struct file_descriptor));
			return i;
		}
	}

	return vector_push(vec, fd);
}

int vector_set(vector_t *vec, struct file_descriptor obj, int fd)
{
	if (fd >= vec->vector_maxsize)
		vector_expand(vec, roundup(fd, vec->vector_maxsize));

	if (fd >= vec->vector_size)
		vec->vector_size = fd + 1;

	memcpy(&vec->vector_buffer[fd], &obj, sizeof(struct file_descriptor));
	return fd;
}