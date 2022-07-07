#ifndef _MM_HEAP_H
#define _MM_HEAP_H

#include <stddef.h>
#include <sys/types.h>
#include <mm/paging.h>

struct heap
{
	offset_t start;    /* Offset in memory to start location of heap */
	size_t   size;     /* The current heap size in bytes */
	size_t   maxsize;  /* The heap's maximum stretch size */
	int      usermode; /* If this is usermode accessable or not */

	struct linkedlist *linkedlist;

	/* This heap structure also acts like a linked list to find the other heaps
	 * created */
	struct heap *nextheap;
};

#define KHEAP_START        0xC0000000
#define KHEAP_MAXSIZE      0x1000000
#define KHEAP_INITIAL_SIZE 0x1000

#define UHEAP_START        0xD0000000
#define UHEAP_MAXSIZE      0x1000000
#define UHEAP_INITIAL_SIZE 0x1000

#define PREHEAPSPACE 0x85000

void *kmalloc_base(size_t size, int align, phys_addr_t *physical_address);
void *kmalloc(size_t size);
int   kfree(void *addr);
void *kcalloc(size_t size, int value);
int   init_kheap();

void *kmalloc_user_base(size_t size, int align, phys_addr_t *physical_address);
void *kmalloc_user(size_t size);
int   kfree_user(void *addr);
int   init_uheap();

void *kmalloc_gen(struct heap *heap, size_t size);
void *kmalloc_gen_base(struct heap *heap, size_t size, int align, phys_addr_t *phys);
int kfree_gen(struct heap *heap, void *addr);

offset_t     get_placement_addr();
struct heap *is_addr_in_heap(offset_t addr);

struct heap *create_heap(struct heap *, offset_t start, size_t maxsize, int usermode);
void heap_setusermode(struct heap *heap);

#endif /* _MM_HEAP_H */