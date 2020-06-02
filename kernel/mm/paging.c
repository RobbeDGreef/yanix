/**
 * mm/paging.c
 *
 * Author: Robbe De Greef
 * Date:   12 may 2019
 *
 * Version 2.0
 */

// @todo: implement COW (copy on write) for duplicating a page see this page for
// more info: https://ubuntuforums.org/showthread.php?t=1308167

#include <cpu/isr.h>
#include <drivers/video/video.h>
#include <errno.h>
#include <kernel.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <proc/tasking.h>
#include <signal.h>
#include <sys/types.h>
#include <yanix/kfunctions.h>
#include <yanix/stack.h>

/**
 * The global directory variables
 */

page_directory_t *         g_kernel_directory  = 0;
volatile page_directory_t *g_current_directory = 0;

/**
 * The global frame variables
 */

offset_t *g_frames;
size_t    g_nframes;

page_directory_t *get_kernel_dir()
{
	return g_kernel_directory;
}

page_directory_t *get_current_dir()
{
	return (page_directory_t *) g_current_directory;
}

void set_current_dir(page_directory_t *new)
{
	g_current_directory = new;
}

extern void page_fault(registers_t *);

/**
 * All the bitset algorithms
 */

// macros used for bitset algorithms
#define INDEX_FROM_BIT(a)  (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

// bitset functions

int get_frame(unsigned int frame_addr)
{
	unsigned int frame  = frame_addr / 0x1000;
	unsigned int index  = INDEX_FROM_BIT(frame);
	unsigned int offset = OFFSET_FROM_BIT(frame);
	return g_frames[index] &= (0x1 << offset);
}

/**
 * @brief      Sets the frame.
 *
 * @param[in]  frame_addr  The frame address
 */
static void set_frame(unsigned int frame_addr)
{
	uint32_t frame  = frame_addr / 0x1000;
	uint32_t index  = INDEX_FROM_BIT(frame);
	uint32_t offset = OFFSET_FROM_BIT(frame);
	g_frames[index] |= (0x1 << offset);
}

/**
 * @brief      Clears a frame.
 *
 * @param[in]  frame_addr  The frame address
 */
static void clear_frame(unsigned int frame_addr)
{
	uint32_t frame  = frame_addr / 0x1000;
	uint32_t index  = INDEX_FROM_BIT(frame);
	uint32_t offset = OFFSET_FROM_BIT(frame);
	g_frames[index] &= ~(0x1 << offset); // '~' bitwise not
}

/**
 * @brief      Finds the first free frame
 *
 * @return     returns the frame index
 */
static ssize_t first_frame()
{
	size_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(g_nframes); i++)
	{
		if (g_frames[i] != 0xFFFFFFFF)
		{
			for (j = 0; j < 32; j++)
			{
				unsigned int toTest = 0x1 << j;
				if (!(g_frames[i] & toTest))
				{
					return (ssize_t) i * 4 * 8 + j;
				}
			}
		}
	}
	return -1;
}

/**
 * @brief      Alocates a frame from a page struct
 *
 * @param      page                        The page struct
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param[in]  is_writable  Indicates if writable
 *
 * @return     successcode
 */
int alloc_frame(page_t *page, int is_kernel, int is_writable_from_userspace)
{
	if (page->frame != 0)
	{
		return -1;
	}
	else
	{
		ssize_t index = first_frame();
		if (index == -1)
		{
			printk(KERN_ERR "NO MEMORY LEFT\n");
			errno = ENOMEM;
			return -2;
		}
		set_frame(index * 0x1000);
		page->present = 1;
		page->rw      = (is_writable_from_userspace) ? 1 : 0;
		page->user    = (is_kernel) ? 0 : 1;
		page->frame   = index;
		return 0;
	}
}

static void set_frame_flags(page_t *page, int kernel, int writeable)
{
	page->rw   = (writeable) ? 1 : 0;
	page->user = (kernel) ? 0 : 1;
}

int flagforce_alloc_frame(page_t *page, int is_kernel, int writeable)
{
	int ret = alloc_frame(page, is_kernel, writeable);

	if (ret == -1)
		set_frame_flags(page, is_kernel, writeable);
	else if (ret == -2)
		return -1;

	return 0;
}

/**
 * @brief      Realocates a frame from a page struct
 *
 * @param      page                        The page struct
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param[in]  is_writable  Indicates if writable
 *
 * @return     successcode
 */
int realloc_frame(page_t *page, int is_kernel, int is_writable_from_userspace)
{
	if (page->frame != 0)
		clear_frame(page->frame);

	ssize_t index = first_frame();

	if (index == -1)
	{
		errno = ENOMEM;
		return -2;
	}

	set_frame(index * 0x1000);
	page->present = 1;
	page->rw      = (is_writable_from_userspace) ? 1 : 0;
	page->user    = (is_kernel) ? 0 : 1;
	page->frame   = index;

	return 0;
}

/**
 * @brief      Allocate virtual memory in current page dir
 *
 * @param[in]  start  The start
 * @param[in]  size   The size
 * @param[in]  user   Whether user mode is enabled or not
 */
void alloc_virt(offset_t start, size_t size, int user)
{
	for (unsigned int i = 0; i <= size; i += 0x1000)
		alloc_frame(get_page(start + i, 1, g_kernel_directory), user ? 0 : 1,
		            user);
}

/**
 * @brief      Maps a frame to requested location
 *
 * @param      page                        The page
 * @param[in]  addr                        The address
 * @param[in]  remap                       Indicates whether it should be
 * remapped if already mapped
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 *
 * @return     success or failure
 */
int map_frame(page_t *page, unsigned int addr, int remap, int is_kernel,
              int is_writable_from_userspace)
{
	if (page->frame != 0 && remap == 0)
	{
		return -1;
	}
	else
	{
		set_frame(addr); // just to be certain the right frame is mapped
		page->present = 1;
		page->rw      = (is_writable_from_userspace) ? 1 : 0;
		page->user    = (is_kernel) ? 0 : 1;
		page->frame   = addr / 0x1000;
		return 0;
	}
}

void free_frame(page_t *page)
{
	uint32_t frame = 0;
	if (!(frame == page->frame))
	{
		return;
	}
	else
	{
		clear_frame(frame);
		page->frame = 0x0;
	}
}

/**
 * @brief      Get the physical address from a virtual one in a page directory
 *
 * @param      virtual_address  The virtual address
 *
 * @return     The physical address
 */
phys_addr_t virt_to_phys_from_dir(void *virtual_address, page_directory_t *dir)
{
	uint32_t page_aligned_addr = (uint32_t) virtual_address & 0xFFFFF000;
	// first get the page
	page_t *page =
		(page_t *) &dir
			->tables[page_aligned_addr / 0x1000 / AMOUNT_OF_PAGES_PER_TABLE]
			->pages[page_aligned_addr / 0x1000 % AMOUNT_OF_PAGES_PER_TABLE];
	if (page->frame == 0)
	{
		return 0;
	}
	return page->frame * 0x1000 + ((uint32_t) virtual_address & 0x00000FFF);
}

/**
 * @brief      Switches page directory
 *
 * @param      dir   The directory to switch to
 */
void switch_page_directory(page_directory_t *dir)
{
	// @bug: so the bug is the stack actually, the stack is completely fcked
	// because we copy it like 5 functions ago we should copy now
	// @fix: copy stack now and exclude it from the copy systems in
	// duplicate_current_page_directory
	g_current_directory = dir;
	asm volatile("mov %0, %%cr3" : : "r"(dir->physicalAddress));
}

/**
 * @brief      Initializes paging
 *
 * @param      dir   The directory to switch to
 */
void init_page_directory(page_directory_t *dir)
{
	switch_page_directory(dir);
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000; // 1000 0000 0000 0000 0000 0000 0001 0001
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

/**
 * @brief      Allocates a page table.
 *
 * @param[in]  table_index  The table index
 * @param      dir          The dir
 */
static void alloc_page_table(size_t table_index, int flags,
                             page_directory_t *dir)
{
	phys_addr_t phys = 0;
	dir->tables[table_index] =
		(page_table_t *) kmalloc_base(sizeof(page_table_t), 1, &phys);
	memset(dir->tables[table_index], 0, sizeof(page_table_t));
	dir->tablesPhysical[table_index] = (uint32_t) phys | flags;
}

/**
 * @brief      Gets the page.
 *
 * @param[in]  virtual_address  The virtual address
 * @param[in]  make_page_table  The make page table
 * @param      dir              The dir
 *
 * @return     The page.
 */
page_t *get_page(uint32_t virtual_address, int make_page_table,
                 page_directory_t *dir)
{
	virtual_address /= 0x1000; // make the address an index

	if (dir->tables[virtual_address / AMOUNT_OF_PAGES_PER_TABLE] == 0
	    && make_page_table)
	{
		// flags are 0x7 binary this is equal to 0111 this means present,
		// read-write, user-mode
		alloc_page_table(virtual_address / AMOUNT_OF_PAGES_PER_TABLE, 0x7, dir);
	}
	else if (dir->tables[virtual_address / AMOUNT_OF_PAGES_PER_TABLE] == 0
	         && !make_page_table)
	{
		return 0;
	}
	return &dir->tables[virtual_address / AMOUNT_OF_PAGES_PER_TABLE]
				->pages[virtual_address
	                    % AMOUNT_OF_PAGES_PER_TABLE]; // return the page
}
#include <debug.h>

void debug_print_phys_frame(offset_t virt_addr, size_t size,
                            page_directory_t *dir)
{
	page_t *toprint = get_page(virt_addr, 0, dir);
	if (!toprint || !toprint->frame)
	{
		printk("No page mapped to print!\n");
		return;
	}
	printk("Frame that is being read: %x\n", toprint->frame);
	page_t *buf = get_page(PAGE_BUFFER_LOCATION, 0,
	                       (page_directory_t *) g_current_directory);
	int     tmp = buf->frame;
	buf->frame  = toprint->frame;
	printk_hd((void *) virt_addr, size);
	printk_hd((void *) PAGE_BUFFER_LOCATION, size);
	buf->frame = tmp;
}
#include <proc/arch_tasking.h>
/**
 * @brief      Copies a page
 *
 * @param[in]  addr    The address of the page
 * @param      newdir  The new directory to copy to
 */
static void copy_page(size_t addr, page_directory_t *newdir)
{
	// printk("\n\n");
	// printk("-");
	page_t *page_to_copy_ref =
		get_page(addr, 1, (page_directory_t *) g_current_directory);
	page_t *buffer_page = get_page(PAGE_BUFFER_LOCATION, 1,
	                               (page_directory_t *) g_current_directory);
	page_t *new_page    = get_page(addr, 1, newdir);

	int bufframe = buffer_page->frame;

	alloc_frame(new_page, page_to_copy_ref->user ? 0 : 1, page_to_copy_ref->rw);

	buffer_page->frame = new_page->frame;
	memcpy((void *) PAGE_BUFFER_LOCATION, (void *) addr, 0x1000);

	buffer_page->frame = bufframe;

	// printk("addr: %x\n", addr);
	arch_flush_tlb();

	// void *ptr = kmalloc(804);
	// kfree(ptr);
}

/**
 * @brief      Duplicates the current page directory
 *
 * @return     Pointer to the duplicate
 */
page_directory_t *duplicate_current_page_directory()
{
	// debug_printk("Gonna copy\n");
	phys_addr_t       phys = 0;
	page_directory_t *newdir =
		(page_directory_t *) kmalloc_base(sizeof(page_directory_t), 1, &phys);
	memset(newdir, 0, sizeof(page_directory_t));
	newdir->physicalAddress =
		(uint32_t) phys
		+ ((uint32_t) newdir->tablesPhysical - (uint32_t) newdir);

	/* loop over all the page tables */
	for (size_t tableiter = 0; tableiter < AMOUNT_OF_PAGE_TABLES_PER_DIR;
	     tableiter++)
	{
		if (g_current_directory->tablesPhysical[tableiter])
		{
			/* if the table is used and allocated */
			if (g_current_directory->tables[tableiter]
			    == g_kernel_directory->tables[tableiter])
			{
				/* if the page is the same as in the kernel directory, then we
				 * should link it */
				newdir->tables[tableiter] =
					g_current_directory->tables[tableiter];
				newdir->tablesPhysical[tableiter] =
					g_current_directory->tablesPhysical[tableiter];
			}
			else
			{
				/* copy the page table */
				newdir->tables[tableiter] = (page_table_t *) kmalloc_base(
					sizeof(page_table_t), 1, &phys);
				memset(newdir->tables[tableiter], 0, sizeof(page_table_t));
				newdir->tablesPhysical[tableiter] =
					(uint32_t) phys
					| (g_current_directory->tablesPhysical[tableiter]
				       & 0xF); // copy the flags over too

				/* loop over evey page and copy the contents if it exists */
				for (size_t pageiter = 0; pageiter < AMOUNT_OF_PAGES_PER_TABLE;
				     pageiter++)
				{
					if (g_current_directory->tables[tableiter]
					        ->pages[pageiter]
					        .present)
					{
						copy_page(
							(tableiter * AMOUNT_OF_PAGES_PER_TABLE + pageiter)
								* 0x1000,
							newdir);
					}
				}
			}
		}
	}
	return newdir;
}

/**
 * @brief      Clears all the frames and page tables in a page directory that
 * are not in use.
 *
 * @param      dir   The page directory
 */
void clear_page_directory(page_directory_t *dir)
{
	for (size_t table = 0; table < AMOUNT_OF_PAGE_TABLES_PER_DIR; table++)
	{
		// if the page table does not pop up in the kernel page directory, we
		// should remove all of its frames
		if (dir->tables[table] != g_kernel_directory->tables[table])
		{
			// free all the frames
			for (size_t page = 0; page < AMOUNT_OF_PAGES_PER_TABLE; page++)
			{
				if (dir->tables[table]->pages[page].frame != 0)
				{
					clear_frame((table * AMOUNT_OF_PAGES_PER_TABLE + page)
					            * 0x1000);
				}
			}
			kfree(dir->tables[table]);
		}
	}
}

/**
 * @brief      Maps a physical memory block to the virtual memory
 *
 * @param[in]  startaddr                   The startaddr
 * @param[in]  endaddr                     The endaddr
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param      dir                         The dir
 *
 * @return     success code
 */
static int map_memory_block(uint32_t startaddr, uint32_t endaddr, int is_kernel,
                            int writeable, page_directory_t *dir)
{
	/* this loop uses startaddr as an iterator */
	while (startaddr < endaddr)
	{
		page_t *page = get_page(startaddr, 1, dir);
		flagforce_alloc_frame(page, is_kernel, writeable);

		startaddr += 0x1000;
	}
	return 0;
}

static int remap_memory_block(uint32_t startaddr, uint32_t endaddr,
                              int is_kernel, int writeable,
                              page_directory_t *dir)
{
	/* this loop uses startaddr as an iterator */
	while (startaddr <= endaddr)
	{
		/* alocates a frame for every page that is in this memory block */
		realloc_frame(get_page(startaddr, 1, dir), is_kernel, writeable);
		startaddr += 0x1000;
	}
	return 0;
}

/**
 * @brief      Maps a physical memory block to the virtual memory
 *
 * @param[in]  startaddr                   The startaddr
 * @param[in]  endaddr                     The endaddr
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 *
 * @return     { description_of_the_return_value }
 */
int map_mem(uint32_t startaddr, uint32_t endaddr, int is_kernel,
            int is_writable_from_userspace)
{
	return map_memory_block(startaddr, endaddr, is_kernel,
	                        is_writable_from_userspace,
	                        (page_directory_t *) g_current_directory);
}

int remap_mem(uint32_t startaddr, uint32_t endaddr, int is_kernel,
              int is_writable_from_userspace)
{
	return remap_memory_block(startaddr, endaddr, is_kernel,
	                          is_writable_from_userspace,
	                          (page_directory_t *) g_current_directory);
}

/**
 * @brief      Identity maps a physical memory block to a virtual memory block
 *
 * @param[in]  startaddr                   The startaddr
 * @param[in]  endaddr                     The endaddr
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param      dir                         The dir
 *
 * @return     success code
 */
int identity_map_memory_block(uint32_t startaddr, uint32_t endaddr,
                              int is_kernel, int is_writable_from_userspace,
                              page_directory_t *dir)
{
	int ret;
	// this loop uses startaddr as an iterator
	while (startaddr <= endaddr)
	{
		/* alocates a frame for every page that is in this memory block */
		ret = map_frame(get_page(startaddr, 1, dir), startaddr, 1, is_kernel,
		                is_writable_from_userspace);
		if (ret != 0)
		{
			return ret;
		}
		startaddr += 0x1000;
	}
	return 0;
}

/**
 * @brief      Maps a physical address block to a virtual one
 *
 * @param      physical_address            The physical address
 * @param      virtual_address             The virtual address
 * @param[in]  size                        The size
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param      dir                         The dir
 *
 * @return     successcode
 */
int map_physical_to_virtual(phys_addr_t *physical_address,
                            void *virtual_address, size_t size, int is_kernel,
                            int               is_writable_from_userspace,
                            page_directory_t *dir)
{
	for (size_t i = 0; i < size; i += 0x1000)
	{
		int ret =
			map_frame(get_page((unsigned int) virtual_address + i, 1, dir),
		              ((uint32_t) physical_address) + i, 1, is_kernel,
		              is_writable_from_userspace);
		if (ret)
		{
			return -1;
		}
	}
	return 0;
}

/**
 * @brief      Initializes paging.
 *
 * @return     exit code
 */
int init_paging()
{
	// @todo: get end of memory
	offset_t end_of_memory = 0xFFFFFFFF;
	g_nframes = end_of_memory / 0x1000; // each page frame coveres 4kib bytes

	g_frames = kmalloc_base(g_nframes / 32, 1, 0);
	memset(g_frames, 0, g_nframes / 32);

	g_kernel_directory =
		(page_directory_t *) kmalloc_base(sizeof(page_directory_t), 1, 0);
	memset(g_kernel_directory, 0, sizeof(page_directory_t));

	g_kernel_directory->physicalAddress =
		(uint32_t) g_kernel_directory->tablesPhysical;

	g_current_directory = g_kernel_directory;

	// we need to create the page tables before the identity paging of the
	// kernel because if we do it afterward we won't be able to call kmalloc
	// anymore since it checks if anything is written to the page direcotry and
	// if so it will use the heap, problem being we haven't initialized our heap
	// yet

	for (uint i = KHEAP_START; i < KHEAP_START + KHEAP_MAXSIZE; i += 0x1000)
		get_page(i, 1, g_kernel_directory);

	// identity map the memory from 0 to the end of the kernel and make it
	// unwriteable from user space and kernel only
	identity_map_memory_block(0, get_placement_addr() + PREHEAPSPACE, 1, 0,
	                          g_kernel_directory);
	// @note: the reason that we need to add so much memory after placement
	// address is because of the page allocation systems

	/* allocate the start of the heaps */
	map_memory_block(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 1, 0,
	                 g_kernel_directory);
	map_memory_block(UHEAP_START, UHEAP_START + UHEAP_INITIAL_SIZE, 0, 1,
	                 g_kernel_directory);

	// allocate the page buffer location for page directory cloning
	alloc_frame(get_page(PAGE_BUFFER_LOCATION, 1, g_kernel_directory), 1, 0);

	// map_memory_block(ZBuffer, ZBuffer+ (MAXBUFFER * MAXWINDOWS) + 0x1000, 0,
	// 1, g_kernel_directory);

	unsigned int buffer_size = video_get_screen_width()
							   * video_get_screen_height()
							   * video_get_screen_bpp();

	// allocate the VESA physical frame buffer bus
	identity_map_memory_block((unsigned int) video_get_screen_fb(),
	                          (unsigned int) video_get_screen_fb()
	                              + buffer_size,
	                          0, 1, g_kernel_directory);

	// register the page interrupt handler
	arch_register_interrupt_handler(14, page_fault);

	// switch the page directory and by doing this activate paging
	g_current_directory = duplicate_current_page_directory();
	init_page_directory((page_directory_t *) g_current_directory);

	return 0;
}

void debug_paging_print(page_directory_t *dir)
{
	/* print important parts of this directory to check if it is corrupted or
	 * not */
	printk("Stack:");
	for (int i = 10; i >= 0; i--)
	{
		int     loc  = DISIRED_STACK_LOCATION - (i * 0x1000);
		page_t *page = get_page(loc, 0, dir);
		printk("loc %x present: %x %i %i\n", loc, page, page ? page->frame : 0,
		       page ? get_frame(page->frame) : 0);
	}
}

void debug_paging_buf(page_directory_t *dir)
{
	page_t *page  = get_page(PAGE_BUFFER_LOCATION, 0, get_current_dir());
	int     frame = get_page(0xFFFF84, 0, dir)->frame;
	printk("frame: %x %x\n", frame, page->frame);
	page->frame = frame;
}