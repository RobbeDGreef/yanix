#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <cpu/isr.h>

#define AMOUNT_OF_PAGES_PER_TABLE 		1024
#define AMOUNT_OF_PAGE_TABLES_PER_DIR 	1024

#define PAGE_BUFFER_LOCATION 0xa00000

typedef struct page {
	uint32_t present	:1;		// page present in memory
	uint32_t rw			:1; 	// read only if clear, readwrite if set
	uint32_t user		:1;		// supervisor level only if clear
	uint32_t accessed	:1;		// has the page been accessed since last refresh
	uint32_t dirty		:1;		// has the page been written to since last refresh
	uint32_t unused		:7;		// unused and reserved bits
	uint32_t frame		:20;	// frame address (shiffted right 12 bits)
} page_t;

typedef struct page_table {
	page_t pages[AMOUNT_OF_PAGES_PER_TABLE];
} page_table_t;

typedef struct page_directory {
	page_table_t 	*tables[AMOUNT_OF_PAGE_TABLES_PER_DIR];
	uint32_t 		tablesPhysical[AMOUNT_OF_PAGE_TABLES_PER_DIR];
	uint32_t 		physicalAddress;
} page_directory_t;

/**
 * @brief      Alocates a frame from a page struct
 *
 * @param      page                        The page struct
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 * @param[in]  is_writable  Indicates if writable
 *
 * @return     success or failure (0/-1)
 */
int alloc_frame(page_t *page, int is_kernel, int is_writable_from_userspace);

/**
 * @brief      Maps a frame to requested location
 *
 * @param      page                        The page
 * @param[in]  addr                        The address
 * @param[in]  remap                       Indicates whether it should be remapped if already mapped
 * @param[in]  is_kernel                   Indicates if kernel
 * @param[in]  is_writable_from_userspace  Indicates if writable from userspace
 *
 * @return     success or failure
 */
int map_frame(page_t *page, unsigned int addr, int remap, int is_kernel, int is_writable_from_userspace);

/**
 * @brief      Get the physical address from a virtual one
 *
 * @param      virtual_address  The virtual address
 *
 * @return     The physical address
 */
phys_addr_t virt_to_phys(void *virtual_address);

/**
 * @brief      Switches page directory
 *
 * @param      dir   The directory to switch to
 */
void switch_page_directory(page_directory_t *dir);

/**
 * @brief      Gets the page.
 *
 * @param[in]  virtual_address  The virtual address
 * @param[in]  make_page_table  The make page table
 * @param      dir              The dir
 *
 * @return     The page.
 */
page_t *get_page(uint32_t virtual_address, int make_page_table, page_directory_t *dir);

/**
 * @brief      Duplicates the current page directory
 *
 * @return     Pointer to the duplicate
 */
page_directory_t *duplicate_current_page_directory();

/**
 * @brief      Initializes paging.
 *
 * @return     exit code
 */
int init_paging();

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
int map_physical_to_virtual(phys_addr_t *physical_address, void *virtual_address, size_t size, int is_kernel,
						    int is_writable_from_userspace, page_directory_t *dir);


/**
 * @brief      Copies the stack to the new address space
 *
 * @param      newdir  The new address space
 */
void copy_stack_to_new_addressspace(page_directory_t *newdir);

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
int map_mem(uint32_t startaddr, uint32_t endaddr, int is_kernel, int is_writable_from_userspace);


/**
 * @brief      Clears all the frames and page tables in a page directory that are not in use.
 *
 * @param      dir   The page directory
 */
void clear_page_directory(page_directory_t *dir);

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
int identity_map_memory_block(uint32_t startaddr, uint32_t endaddr, int is_kernel, int is_writable_from_userspace,
									 page_directory_t *dir);

#endif