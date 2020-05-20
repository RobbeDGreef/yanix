#include <libk/string.h>
#include <sys/types.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <kernel.h>
#include <mm/linkedlist.h>

#include <debug.h>

/* Defined in linker.ld, just a pointer to the end of memory */
extern uint32_t end_of_bin;
volatile uint32_t placement_address = (uint32_t) &end_of_bin;

/**
 * @brief      Double linked list block heap
 */
struct heap *kernel_heap = 0;
struct heap *user_heap = 0;

/* Simple pointer to the first heap ever created */
struct heap *heaplist = 0;

void print_list(struct ll_node*);

/**
 * @note       This is a debugging function
 * @brief      Prints all the blocks in the used list.
 */
void print_list_used()
{
	print_list(kernel_heap->linkedlist->start_used);
}

/**
 * @brief      Simple watermark allocator, simply allocates memory after kernel bin
 *
 * @param[in]  size   The size
 * @param[in]  align  Wheter the memory should be aligned or not
 *
 * @return     Pointer to the memory
 */
void *simple_alloc(size_t size, int align)
{
	if (align)
	{
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}

	placement_address += size;
	return (void*) (placement_address - size);
}

void *alloc(struct heap *heap, size_t size, int align)
{
	struct ll_node *freenode = get_free(size, heap->linkedlist, align);

	if (!freenode)
		return 0;

	return (void*) ((offset_t) freenode + sizeof(struct ll_node));
}

int free(struct heap *heap, void *memory)
{
	return free_node((struct ll_node*) (memory - sizeof(struct ll_node)), heap->linkedlist);
}

void *kmalloc_base(size_t size, int align, phys_addr_t *physical_address)
{
	if (kernel_heap)
	{
		void *addr = alloc(kernel_heap, size, align);

		if (!addr)
		{
			printk(KERN_ERR "Kernel could not allocate: %i amount of memory\n", size);
			return 0;
		}

		if (physical_address)
		{
			page_t *page = get_page((offset_t) addr, 1, get_kernel_dir());

			/* Sometimes the page won't be initialised yet 
			 * so we'll do that here */
			if (!page->frame)
				alloc_frame(page, 1, 0);

			*physical_address = page->frame * 0x1000 + ((offset_t) addr & 0xFFF);
		}

		return addr;
	}

	void *addr = simple_alloc(size, align);

	if (physical_address)
		*physical_address = (phys_addr_t) addr;

	return addr;
}

void *kmalloc_user_base(size_t size, int align, phys_addr_t *physical_address)
{
	void *addr = alloc(user_heap, size, align);

	if (!addr)
	{
		printk(KERN_ERR "Kernel could not allocate: %i amount of memory\n", size);
		return 0;
	}

	if (physical_address)
		{
			page_t *page = get_page((offset_t) addr, 1, get_kernel_dir());

			/* Sometimes the page won't be initialised yet 
			 * so we'll do that here */
			if (!page->frame)
				alloc_frame(page, 0, 1);

			*physical_address = page->frame * 0x1000 + ((offset_t) addr & 0xFFF);
		}

	return addr;
}

void *kmalloc(size_t size)
{
	return kmalloc_base(size, 0, 0);
}

void *kcalloc(size_t size, int value)
{
	void *tmp = kmalloc_base(size, 0, 0);
	memset(tmp, value, size);
	return tmp;
}

void *kmalloc_user(size_t size)
{
	return kmalloc_user_base(size, 0, 0);
}

int kfree(void *addr)
{
	return free(kernel_heap, addr);
}

int kfree_user(void *addr)
{
	return free(user_heap, addr);
}


/**
 * @brief      Gets the placementaddress of the watermark allocator.
 *
 * @return     The placementaddress.
 */
offset_t get_placement_addr()
{
	return placement_address;
}

/**
 * @brief      Determines whether the specified address is address in a heap.
 *
 * @param[in]  addr  The address
 *
 * @return     True if the specified address is address in heap, False otherwise.
 */
struct heap *is_addr_in_heap(offset_t addr)
{
	struct heap *tmp = heaplist;

	if (!tmp)
		return 0;

	while (tmp != 0)
	{
		if (addr > tmp->start && addr < tmp->start+tmp->maxsize)
			return tmp;

		tmp = tmp->nextheap;
	}

	return 0;
}

static void add_heap(struct heap *heap)
{
	heap->nextheap = 0;

	if (!heaplist)
	{
		heaplist = heap;
		return;
	}

	struct heap *tmp = heaplist;
	
	while (tmp->nextheap)
		tmp = tmp->nextheap;

	tmp->nextheap = heap;
}

static struct heap* create_heap(offset_t start, size_t maxsize, int usermode)
{
	struct heap *heap = simple_alloc(sizeof(struct heap), 0);

	heap->start = start;
	heap->size  = 0;
	heap->maxsize = maxsize;
	heap->usermode = usermode;

	heap->linkedlist = create_linkedlist(start, maxsize, usermode);

	add_heap(heap);

	return heap;
}

/**
 * @brief      Initializes the kernel heap.
 *
 * @return     Always 0
 */
int init_kheap()
{
	kernel_heap = create_heap(KHEAP_START, KHEAP_MAXSIZE, 0);
	return 0;
}

/**
 * @brief      Initializes the user heap.
 *
 * @return     Always 0
 */
int init_uheap()
{
	user_heap = create_heap(UHEAP_START, UHEAP_MAXSIZE, 1);
	return 0;
}