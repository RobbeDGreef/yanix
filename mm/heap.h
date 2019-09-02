#ifndef HEAP_H
#define HEAP_H

#include <mm/blockArray.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <include/sys/types.h>

#define KHEAP_START			0xC0000000
#define KHEAP_MAXSIZE		0x100000
#define KHEAP_MAXINDEX		0x1000
#define KHEAP_INITIAL_SIZE	0x10000

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
void init_kheap();

/**
 * @brief      Frees a block of memory
 *
 * @param      addr  The address of the memory block
 *
 * @return     success of failure
 */
int kfree(void *addr);
#endif