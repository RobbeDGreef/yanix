/**
 * kernel/execute/elf.c
 *
 * Author: Robbe De Greef
 * Date:   24 may 2019
 *
 * Version 1.0
 */

#include <errno.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <proc/tasking.h>
#include <stddef.h>
#include <stdint.h>
#include <yanix/elf.h>

#define ELF_PHDR_NULL    0
#define ELF_PHDR_LOAD    1
#define ELF_PHDR_DYNAMIC 2
#define ELF_PHDR_INTERP  3
#define ELF_PHDR_NOTE    4
#define ELF_PHDR_SHLIB   5
#define ELF_PHDR_PHDR    6
#define ELF_PHDR_LOPROC  0x70000000
#define ELF_PHDR_HIPROC  0x7fffffff

#define ELF_SHDR_WRITE     0x1
#define ELF_SHDR_ALLOC     0x2
#define ELF_SHDR_EXECINSTR 0x4
#define ELF_SHDR_MASKPROC  0xf0000000

#define ELF_R 4
#define ELF_W 2
#define ELF_X 1

#define ELF_SHDR_TYPE_NULL     0
#define ELF_SHDR_TYPE_PROGBITS 1
#define ELF_SHDR_TYPE_SYMTAB   2
#define ELF_SHDR_TYPE_STRTAB   3
#define ELF_SHDR_TYPE_RELA     4
#define ELF_SHDR_TYPE_HASH     5
#define ELF_SHDR_TYPE_DYNAMIC  6
#define ELF_SHDR_TYPE_NOTE     7
#define ELF_SHDR_TYPE_NOBITS   8
#define ELF_SHDR_TYPE_REL      9
#define ELF_SHDR_TYPE_SHLIB    10
#define ELF_SHDR_TYPE_DYNSYM   11

#define ELF_SHDR_TYPE_LOPROC 0x70000000
#define ELF_SHDR_TYPE_HIPROC 0x7fffffff
#define ELF_SHDR_TYPE_LOUSER 0x80000000
#define ELF_SHDR_TYPE_HIUSER 0xffffffff

/**
 * @brief      Checks elf file support
 *
 * @param      hdr   The header
 *
 * @return     { description_of_the_return_value }
 */
static int elf_check_support(elf32_hdr_t *hdr)
{
	if (hdr->magic != ELF_MAGIC)
	{
		/* not an elf file */
		errno = ENOEXEC;
		return -1;
	}
	else if (hdr->bit_type != ELF_BIT32)
	{
		// 32 bit not supported
		errno = ENOEXEC;
		return -1;
	}
	else if (hdr->arch_type != ELF_ARCH_X86)
	{
		// not 32 bit target
		errno = ENOEXEC;
		return -1;
	}
	else if (hdr->elf_header_version < ELF_SUPPORTED)
	{
		// elf header not supported
		errno = ENOEXEC;
		return -1;
		//} else if (hdr->type != ELF_TYPE_REL && hdr->type != ELF_TYPE_EXEC){
	}
	else if (hdr->type != ELF_TYPE_EXEC)
	{
		// elf file type not supported
		errno = ENOEXEC;
		return -1;
	}
	return 0;
}

#include <debug.h>

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
	memset((void *) phdr->vaddr, 0, phdr->memsize);
	memcpy((void *) phdr->vaddr, (void *) ((unsigned int) file + phdr->offset),
	       phdr->filesize);
	if (phdr->filesize < phdr->memsize)
		memset((void *) (phdr->vaddr + phdr->filesize), 0,
		       phdr->memsize - phdr->filesize);

	return 0;
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
static int _elf_loop_over_program_table(void *file, elf32_hdr_t *elf_hdr,
                                        elf32_phdr_t *elf_program_table)
{
	for (size_t i = 0; i < elf_hdr->pheader_table_amount; i++)
	{
		if (elf_program_table[i].filesize > elf_program_table[i].memsize)
		{
			errno = ENOEXEC;
			return -1;
		}
		else if (elf_program_table[i].type == ELF_PHDR_NULL)
		{
			continue;
		}
		else if (elf_program_table[i].type == ELF_PHDR_LOAD)
		{
			if (_elf_load_pheader(file, &elf_program_table[i]) == -1)
			{
				return -1;
			}
		}
	}

	/* Find the program break */

	unsigned long pbrk =
		elf_program_table[0].vaddr + elf_program_table[0].memsize;
	unsigned long pstrt = elf_program_table[0].vaddr;

	for (size_t i = 1; i < elf_hdr->pheader_table_amount; i++)
	{
		if ((elf_program_table[i].vaddr + elf_program_table[i].memsize) > pbrk)
			pbrk = elf_program_table[i].vaddr + elf_program_table[i].memsize;

		if ((elf_program_table[i].vaddr) < pstrt)
			pstrt = (elf_program_table[i].vaddr);
	}

	get_current_task()->program_break = pbrk;
	get_current_task()->program_start = pstrt;

	return 0;
}

static int _elf_loop_over_section_table(elf32_hdr_t * elf_hdr,
                                        elf32_shdr_t *elf_section_table)
{
	(void) (elf_section_table);

	for (size_t i = 0; i < elf_hdr->sheader_table_amount; i++)
	{
		/* ??? */
	}
	return 0;
}

uint32_t load_elf_into_mem(void *file)
{
	elf32_hdr_t *elf_hdr = (elf32_hdr_t *) file;
	if (elf_check_support(elf_hdr) == -1)
		return 0;

	// locating the header tables
	elf32_shdr_t *elf_section_table =
		(elf32_shdr_t *) ((uint32_t) file + elf_hdr->sheader_table_position);
	elf32_phdr_t *elf_program_table =
		(elf32_phdr_t *) ((uint32_t) file + elf_hdr->pheader_table_position);

	// looping over the header tables
	if (_elf_loop_over_section_table(elf_hdr, elf_section_table) == -1)
		return 0;

	if (_elf_loop_over_program_table(file, elf_hdr, elf_program_table) == -1)
		return 0;

	return elf_hdr->entry;
}