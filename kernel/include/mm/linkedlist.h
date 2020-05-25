#ifndef _MM_LINKEDLIST_H
#define _MM_LINKEDLIST_H

#include <stddef.h>
#include <sys/types.h>

#define NODESIZE_MAX 32

struct ll_node
{
	size_t size; /* The size of this block (without the size of the this node
	                added to it) */

	struct ll_node *next;
	struct ll_node *prev;
};

struct linkedlist
{
	offset_t memstart; /* The start of the heap in memory */
	size_t   maxsize;  /* The maximum heap size in bytes */
	size_t   cursize;  /* The current size of the heap in bytes (end address =
	                      start address + cursize) */
	int user;

	struct ll_node *start_used;
	struct ll_node *start_free;
};

struct linkedlist *create_linkedlist(offset_t memorystart, size_t maxsize,
                                     int user);

int             free_node(struct ll_node *node, struct linkedlist *list);
struct ll_node *get_free(size_t searchsize, struct linkedlist *list,
                         int pagealigned);

#endif /* _MM_LINKEDLIST_H */