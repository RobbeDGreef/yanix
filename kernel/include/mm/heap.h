#ifndef HEAP_H
#define HEAP_H

#include <mm/blockArray.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#define KHEAP_START			0xC0000000
#define KHEAP_MAXSIZE		0x100000
#define KHEAP_MAXINDEX		0x1000
#define KHEAP_INITIAL_SIZE	0x30000			/* Depricated */

#define UHEAP_START 		0xD0000000
#define UHEAP_MAXSIZE		0x100000
#define UHEAP_MAXINDEX		0x500
#define UHEAP_INITIAL_SIZE 	0x20000

#define PREHEAPSPACE 	0x85000

typedef struct {
	blockArray *array;
	uint32_t 	start;
	uint32_t 	size;
	uint32_t 	maxsize;
	int 	 	user;
} heap_t;

/**
 * @brief      Extensive kernel malloc function
 *
 * @param[in]  size             The size
 * @param[in]  aligned          The aligned
 * @param      physicaladdress  The physicaladdress
 *
 * @return     Pointer to memory
 */
void *kmalloc_base(size_t size, int aligned, phys_addr_t *physicaladdress);


/**
 * @brief      Simple kernel kcalloc function
 *
 * @param[in]  size   The size
 * @param[in]  value  The value
 *
 * @return     Pointer to initialised memory
 */
void *kcalloc(size_t size, int value);

/**
 * @brief      Simple kernel kmalloc function
 *
 * @param[in]  size  The size
 *
 * @return     Pointer to memory
 */
void *kmalloc(size_t size);

/**
 * @brief      Initializes the kernel heap
 */
int init_kheap();

/**
 * @brief      Initialises the user heap
 */
int init_uheap();


/**
 * @brief      Frees a block of memory
 *
 * @param      addr  The address of the memory block
 *
 * @return     success of failure
 */
int kfree(void *addr);

/**
 * @brief      Frees up a previously allocated block of memory on the heap
 *
 * @param      addr  The startaddress of the block
 *
 * @return     success of failure (0/-1)
 */
int kfree_user(void *addr);

/**
 * @brief      Allocates a block of memory on the stack (simple)
 *
 * @param[in]  size  The size of the block
 *
 * @return     A pointer to the block of memory
 */
void *kmalloc_user(size_t size);

/**
 * @brief      Allocates a block of memory on the user heap
 *
 * @param[in]  size             The size
 * @param[in]  aligned          The aligned
 * @param      physicaladdress  The physicaladdress
 *
 * @return     Pointer to the allocated block
 */
void *kmalloc_user_base(size_t size, int aligned, phys_addr_t *physicaladdress);

void heapstatus(heap_t *heap, uint32_t location);

#endif