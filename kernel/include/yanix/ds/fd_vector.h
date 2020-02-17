#ifndef _YANIX_DS_FD_VECTOR_H
#define _YANIX_DS_FD_VECTOR_H

#include <fs/filedescriptor.h>

#define VECTOR_BEGIN_AMOUNT 8

typedef struct vector_s
{
	struct file_descriptor	*vector_buffer;		/* Pointer to the current vector array buffer */
	int 					vector_size;		/* The amount of vector entries */
	int 					vector_maxsize;		/* The maximum amount of entries that can be added without resizing the vector */
} vector_t;

vector_t *vector_create();
vector_t *vector_copy(vector_t *cp);
int vector_clear(vector_t *vec, int fd);
int vector_add(vector_t *vec, struct file_descriptor fd);
struct file_descriptor vector_get(vector_t *vec, int fd);

#endif /* _YANIX_DS_FD_VECTOR_H */