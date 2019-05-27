/**
 * kernel/execute/elf.c
 * 
 * Author: Robbe De Greef
 * Date:   24 may 2019
 * 
 * Version 1.0
 */

#include <kernel/execute/elf.h>
#include <mm/paging.h>
#include <lib/string/string.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>


#define ELF_PHDR_NULL		0
#define ELF_PHDR_LOAD		1
#define ELF_PHDR_DYNAMIC	2
#define ELF_PHDR_INTERP		3
#define ELF_PHDR_NOTE		4
#define ELF_PHDR_SHLIB		5
#define ELF_PHDR_PHDR		6
#define ELF_PHDR_LOPROC 	0x70000000
#define ELF_PHDR_HIPROC		0x7fffffff

#define ELF_R 	4
#define ELF_W	2
#define ELF_X 	1

/**
 * @brief      Checks elf file support
 *
 * @param      hdr   The header
 *
 * @return     { description_of_the_return_value }
 */
static int elf_check_support(elf32_hdr_t *hdr)
{
	if (hdr->magic != ELF_MAGIC) {
		// not an elf file
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->bit_type != ELF_BIT32) {
		// 32 bit not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->arch_type != ELF_ARCH_X86) {
		// not 32 bit target
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->elf_header_version < ELF_SUPPORTED) {
		// elf header not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	//} else if (hdr->type != ELF_TYPE_REL && hdr->type != ELF_TYPE_EXEC){
	} else if (hdr->type != ELF_TYPE_EXEC) {
		// elf file type not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	}
	return 0;
}

/**
 * @brief      Loads a program header into memory
 *
 * @param      file  The file pointer
 * @param      phdr  The program header
 *
 * @return     success
 */
static int _elf_load_pheader(void *file, elf32_phdr_t *phdr)
{
	map_mem(phdr->vaddr, phdr->vaddr + phdr->memsize, 0, phdr->flags & ELF_W);
	memset((void*) phdr->vaddr,0, phdr->memsize);
	memcpy((void*) phdr->vaddr, (void*) ((uint32_t) file + phdr->offset), phdr->filesize);
	if (phdr->filesize < phdr->memsize) {
		memset((void*)(phdr->vaddr + phdr->filesize), 0, phdr->memsize-phdr->filesize);
	}
	return  0; 
}

/**
 * @brief      Loops over the program header
 *
 * @param      file    The file pointer
 * @param      phdr    The program header
 * @param[in]  size    The size of a program header entry
 * @param[in]  amount  The amount of program header entries
 *
 * @return     success
 */
static int _elf_loop_over_pheader(void *file, elf32_phdr_t* phdr, size_t size, size_t amount)
{
	for (size_t i = 0; i < amount; i++){
		
		if (phdr->filesize > phdr->memsize) {
			errno = ENOEXEC;
			return -1;
		} else if (phdr->type == ELF_PHDR_NULL) {
			continue;
		} else if (phdr->type == ELF_PHDR_LOAD) {
			if (_elf_load_pheader(file, phdr) == -1){
				return -1;
			}
		}
		phdr = (elf32_phdr_t*) ((uint32_t)phdr+size);
	}
	return 0;

}


/**
 * @brief      Loads an elf into memory.
 *
 * @param      file  The file
 *
 * @return     successcode
 */
uint32_t load_elf_into_mem(void* file) 
{
	// check whether it is executable
	elf32_hdr_t *hdr = (elf32_hdr_t*) file;
	if (elf_check_support(hdr) == -1) {
		return 0;
	}
	elf32_phdr_t *phdr = (elf32_phdr_t*) (file+hdr->pheader_table_position);

	if (_elf_loop_over_pheader(file, phdr, hdr->pheader_entry_size, hdr->pheader_table_amount) == -1){
		return 0;
	}

	return hdr->entry;
}