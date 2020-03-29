#include <mm/linkedlist.h>
#include <mm/paging.h>
#include <sys/types.h>
#include <errno.h>

#include <debug.h>

/**
 * @todo: I should look into another system because of all the intern fragmentation this system will cause
 */

/**
 * @optimize: 	- I don't think the used list should even exist lol
 * 				- The used list should not be sorted like it is right now
 *     			- If we remove free_node's used check we could speed up freeing memory
 */

/* From mm/heap.h */
void *simple_alloc(size_t size, int align);

int listcount(struct ll_node *start)
{
	if (start == 0)
		return 0;
	int cnt = 1;
	while ((start = start->next) != 0)
		cnt++;
	return cnt;
}


void print_list(struct ll_node *node)
{
	debug_print("\nLIST:");
	do
	{
		debug_print_hex((int) node); debug_print(" - "); debug_print_hex((int) node + node->size); debug_print(" | ");
	}
	while ((node = node->next) != 0);
	debug_print("\n");
}

struct linkedlist *create_linkedlist(offset_t memorystart, size_t maxsize, int user)
{
	struct linkedlist *heap = simple_alloc(sizeof(struct linkedlist), 0);

	heap->memstart = memorystart;
	heap->maxsize  = maxsize;
	heap->cursize  = 0;
	heap->user 	   = user;

	heap->start_used = heap->start_free = 0;

	return heap;
}


void add_to_used(struct ll_node *node, struct linkedlist *list)
{
	struct ll_node *tmp = list->start_used;

	if (!tmp)
	{
		list->start_used = node;
		node->next = node->prev = 0;
		return;
	}

	while (tmp->next){
		//debug_printk("Next: %x and loc %x\n", tmp->next, &tmp->next);
		tmp = tmp->next;
	}

	tmp->next = node;
	node->prev = tmp;
	node->next = 0;
}

void add_to_free(struct ll_node *node, struct linkedlist *list)
{
	struct ll_node *tmp = list->start_free;

	if (!tmp)
	{
		list->start_free = node;
		node->next = node->prev = 0;
		return;
	}

	/**
	 * I know this is a very weird loop but it's late and this works, i just wrote it weird.
	 * 
	 * @todo        Look into cleaning this code
	 */
	do
	{
		if (tmp->size > node->size)
		{
			if (tmp->prev)
				tmp->prev->next = node;
			
			else
				list->start_free = node;

			node->prev = tmp->prev;
			tmp->prev = node;
			node->next = tmp;

			return;
		}
		if (tmp->next == 0)
		{
			break;
		}
		
		tmp = tmp->next;
	} while (1);

	tmp->next = node;
	node->prev = tmp;
	node->next = 0;
}

void mark_used(struct ll_node *node, struct linkedlist *list)
{
	/* Release node from free list */
	if (node->prev)
		node->prev->next = node->next;
	
	else
		list->start_free = node->next;

	if (node->next)
		node->next->prev = node->prev;

	add_to_used(node, list);
}

void mark_free(struct ll_node *node, struct linkedlist *list)
{
	/* Release node from used list */
	if (node->prev)
		node->prev->next = node->next;

	else
		list->start_used = node->next;

	if (node->next)
		node->next->prev = node->prev;

	add_to_free(node, list);
}

offset_t next_page_address(offset_t mem)
{
	return (mem & 0xFFFFF000) + 0x1000;
}

static inline struct ll_node *expand_heap_align_split(size_t nodesize, offset_t heapend, offset_t alignoffset, struct linkedlist *list)
{
	struct ll_node *lnode = (struct ll_node*) heapend;
	lnode->size = alignoffset - (sizeof(struct ll_node) * 2);
	lnode->prev = lnode->next = 0;
	add_to_free(lnode, list);

	struct ll_node *newnode = (struct ll_node*) (heapend + alignoffset - sizeof(struct ll_node));
	newnode->size = nodesize;
	newnode->prev = newnode->next = 0;

	list->cursize += alignoffset + nodesize;

	return newnode;
}

struct ll_node *expand_heap(size_t nodesize, struct linkedlist *list, int pagealign)
{
	if (pagealign)
	{
		offset_t addr = next_page_address(list->memstart + list->cursize);
		size_t offset = addr - (list->memstart + list->cursize);
		

		/**
		 * expand until new addr,
		 * check if expanded size is large enough for a left node and a new node
		 * if not make hole until next page
		 */
		if (offset + nodesize + list->cursize <= list->maxsize)
		{
			if (offset >= sizeof(struct ll_node) * 2 + NODESIZE_MAX)
			{
				/* Create a left node and a new node */
				return expand_heap_align_split(nodesize, list->memstart + list->cursize, offset, list);
			}
			else if (offset == sizeof(struct ll_node))
			{
				/* Only create one node */
				
				struct ll_node *newnode = (struct ll_node*) (list->memstart + list->cursize);
				newnode->size = nodesize;
				newnode->prev = newnode->next = 0;

				list->cursize += nodesize + offset;
				return newnode;
			}
			else
			{
				/* Use next page aligned address and create large new left node */
				return expand_heap_align_split(nodesize, list->memstart + list->cursize, offset + 0x1000, list);
			}
		}
	}
	else
	{
		if (list->cursize + nodesize + sizeof(struct ll_node) <= list->maxsize)
		{
			struct ll_node *newnode = (struct ll_node*) (list->memstart + list->cursize);

			newnode->size = nodesize;
			newnode->next = newnode->prev = 0;
			
			list->cursize += nodesize + sizeof(struct ll_node);

			return newnode;
		}
	}

	errno = ENOMEM;
	return 0;
}

struct ll_node *perfect_fit(struct ll_node *node, struct linkedlist *list)
{
	mark_used(node, list);

	return node;
}

struct ll_node *split_fit(size_t nodesize, struct ll_node *node, struct linkedlist *list)
{

	/* If we can't create a whole new block out of it, just let the block be larger */
	if (node->size - nodesize < NODESIZE_MAX + sizeof(struct ll_node))
	{
		mark_used(node, list);
		return node;
	}
	else
	{
		struct ll_node *newnode = (struct ll_node*) ((offset_t) node + nodesize + sizeof(struct ll_node)); 
		newnode->size = node->size - (nodesize + sizeof(struct ll_node));
		newnode->next = newnode->prev = 0;

		node->size = nodesize;

		mark_used(node, list);
		add_to_free(newnode, list);

		return node;
	}
	return 0;
}

offset_t check_pa_addr_in(struct ll_node *node, size_t size)
{
	(void) (node);
	(void) (size);

	return 0;
}

struct ll_node *get_free(size_t searchsize, struct linkedlist *list, int pagealigned)
{

	if (searchsize < NODESIZE_MAX)
		searchsize = NODESIZE_MAX;

	struct ll_node *tmp = list->start_free;

	if (!tmp)
		goto expand;
	
	if (pagealigned)
	{
		do
		{
			if (tmp->size >= searchsize)
			{
				if (!(((offset_t) tmp + sizeof(struct ll_node)) & 0xFFF))
				{
					debug_print("Does this ever happen really ?\n");
					/* Start of block is page aligned */
					if (tmp->size != searchsize)
						return split_fit(searchsize, tmp, list);
					
					else
						return perfect_fit(tmp, list);
				}
				
				/* @warning    THIS MIGHT BE BUGGY */
				/* @todo       I actually turned this system off by returning null in check_pa_addr_in because i think this is just tooo buggy */

				continue;

				/* Returns a page aligned address in a block (if there is any) */
				offset_t addr = check_pa_addr_in(tmp, searchsize+sizeof(struct ll_node));
				if (addr)
				{
					/* First cut off until new addr */
					size_t prevsize = tmp->size;
					tmp->size = (addr - sizeof(struct ll_node)) - ((offset_t) tmp + sizeof(struct ll_node));

					struct ll_node *newnode = (struct ll_node*) (addr - sizeof(struct ll_node));
					newnode->size = searchsize;
					newnode->prev = newnode->next = 0;

					add_to_used(newnode, list);

					if (((offset_t) tmp + sizeof(struct ll_node) + prevsize) - (addr + searchsize) > NODESIZE_MAX + sizeof(struct ll_node))
					{
						/* Append another block at the end */
						struct ll_node *endnode = (struct ll_node*) (addr + searchsize);
						endnode->size = (offset_t) tmp + sizeof(struct ll_node) + prevsize - (addr + searchsize + sizeof(struct ll_node));

						add_to_free(endnode, list);
					}
					else
						newnode->size = prevsize - sizeof(struct ll_node) - tmp->size;

					return newnode;
				}
			}
		} while ((tmp = tmp->next) != 0);
	}
	else
	{
		do
		{
			if (tmp->size >= searchsize)
			{
				if (tmp->size != searchsize)
					return split_fit(searchsize, tmp, list);
				
				else
					return perfect_fit(tmp, list);
			}

		} while ((tmp = tmp->next) != 0);
	}

	/* Expand the heap */
expand:;
	struct ll_node *newnode = expand_heap(searchsize, list, pagealigned);
	add_to_used(newnode, list);
	return newnode;
}

int free_node(struct ll_node *node, struct linkedlist *list)
{
	/**
	 * The only reason we keep the start used list and check if the node is in that
	 * list here is because of safety, we could remove these checks and speed up 
	 * this whole memory manager but it would make the heap so much less safe and prone
	 * to corruptions.
	 */

	struct ll_node *tmp = list->start_used;

	if (!tmp)
		goto error;
	
	do
	{
		if ((offset_t) tmp == (offset_t) node)
		{
			mark_free(node, list);
			return 0;
		}
	
	} while ((tmp = tmp->next) != 0);

error:
	errno = EFAULT;
	return -1;
}