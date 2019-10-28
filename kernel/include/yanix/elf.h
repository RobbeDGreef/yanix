#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>

#define ELF_MAGIC       0x464c457f 	// 0x7f 'E' 'L' 'F'
#define ELF_SUPPORTED   1

#define ELF_BIT32 1
#define ELF_BIT64 2

#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN    2

#define ELF_ARCH_NO_SPECIFIC	0x00
#define ELF_ARCH_SPARC			0x02
#define ELF_ARCH_X86			0x03
#define ELF_ARCH_MIPS			0x08
#define ELF_ARCH_POWERPC		0x14
#define ELF_ARCH_ARM			0x28
#define ELF_ARCH_SUPERH			0x2A
#define ELF_ARCH_IA64			0x32
#define ELF_ARCH_X86_64			0x3E
#define ELF_ARCH_AARCH64		0xB7

#define ELF_CURRENT_VERSION     1

#define ELF_TYPE_NONE   0
#define ELF_TYPE_REL    1
#define ELF_TYPE_EXEC   2


#define ELF_RELOC_ERR  -1
#define ELF_UNDEF       0

#define ELF_SECTION_LORESERVE   0xff00
#define ELF_SECTION_LOPROC      0xff00
#define ELF_SECTION_HIPROC      0xff1f
#define ELF_SECTION_ABS         0xfff1
#define ELF_SECTION_COMMON      0xfff2
#define ELF_SECTION_HIRESERVE   0xffff

typedef struct elf32_hdr_s {
	uint32_t 	magic;
	uint8_t 	bit_type; 		// (32 bit / 64 bit)
	uint8_t  	endian;
    uint8_t  	elf_header_version;
    uint8_t  	abi;
    uint8_t  	abi_version;
    uint8_t  	pad[7];
    uint16_t 	type;			// 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t 	arch_type;
    uint32_t 	elf_version;
    uint32_t 	entry;
    uint32_t 	pheader_table_position;
    uint32_t 	sheader_table_position;
    uint32_t 	flags;
    uint16_t 	header_size;
    uint16_t 	pheader_entry_size;
    uint16_t 	pheader_table_amount;
    uint16_t 	sheader_entry_size;
    uint16_t 	sheader_table_amount;
	uint16_t 	sheader_str_names_index;

}__attribute__((packed)) elf32_hdr_t;

typedef struct elf32_phdr_s {
	uint32_t 	type;
	uint32_t 	offset;
	uint32_t 	vaddr;
	uint32_t 	paddr;
	uint32_t 	filesize;
	uint32_t	memsize;
	uint32_t	flags;
	uint32_t	align;

}__attribute__((packed)) elf32_phdr_t;

typedef struct elf32_shdr_s {
    uint32_t    name;
    uint32_t    type;
    uint32_t    flags;
    uint32_t    addr;
    uint32_t    offset;
    uint32_t    size;
    uint32_t    link;
    uint32_t    info;
    uint32_t    addr_align;
    uint32_t    entry_size;
}__attribute__((packed)) elf32_shdr_t;


/**
 * @brief      Loads an elf into memory.
 *
 * @param      file  The file
 *
 * @return     successcode
 */
uint32_t load_elf_into_mem(void* file);


#endif