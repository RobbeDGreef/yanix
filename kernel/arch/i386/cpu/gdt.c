#include <libk/string.h>
#include <stdint.h>

/**
 * @brief      The x86 GDT entry structure
 */
struct gdt_entry_s {
   uint16_t       limit_low;
   uint16_t       base_low;
   uint8_t        base_middle;
   uint8_t        access;
   uint8_t        granularity;
   uint8_t        base_high;
}__attribute__((packed));

typedef struct gdt_entry_s gdt_entry_t;


/**
 * @brief      The x86 GDT pointer strucutre
 */
struct gdt_ptr_s {
   uint16_t       limit;  
   uint32_t       base;    // address of fist gdt_entry_t struct
} __attribute__((packed));

typedef struct gdt_ptr_s gdt_ptr_t;

/**
 * @brief      The x86 TSS entry structure 
 */
struct tss_entry_s {
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         // The value to load into ES when we change to kernel mode.
   uint32_t cs;         // The value to load into CS when we change to kernel mode.
   uint32_t ss;         // The value to load into SS when we change to kernel mode.
   uint32_t ds;         // The value to load into DS when we change to kernel mode.
   uint32_t fs;         // The value to load into FS when we change to kernel mode.
   uint32_t gs;         // The value to load into GS when we change to kernel mode.
   uint32_t ldt;        
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));

typedef struct tss_entry_s tss_entry_t; 


/* asm function to flush gdt and tss */
extern void gdt_flush(uint32_t);
extern void tss_flush();

/* Global GDT and TSS variables */
gdt_entry_t 	gdt_entries[6];
gdt_ptr_t 		gdt_ptr;

tss_entry_t 	tss_entry;

/**
 * @brief      Sets a GDT gate
 *
 * @param[in]  num     The number
 * @param[in]  base    The base
 * @param[in]  limit   The limit
 * @param[in]  access  The access
 * @param[in]  gran    The gran
 */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	gdt_entries[num].base_low 		= (base & 0xFFFF);
	gdt_entries[num].base_middle 	= (base >> 16) & 0xFF;
	gdt_entries[num].base_high 		= (base >> 24) & 0xFF;

	gdt_entries[num].limit_low 		= (limit & 0xFFFF);
	gdt_entries[num].granularity 	= (limit >> 16) & 0x0F;

	gdt_entries[num].granularity 	|= gran & 0xF0;
	gdt_entries[num].access 		= access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0)
{
	uint32_t base  = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	gdt_set_gate(num, base, limit, 0xE9, 0x00);

	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0  = ss0;
	tss_entry.esp0 = esp0;

	tss_entry.cs = 0x0B;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
	tss_entry.iomap_base = sizeof(tss_entry);
}


static void init_gdt()
{
	gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
	gdt_ptr.base  = (uint32_t) &gdt_entries;

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	write_tss(5, 0x10, 0x0);

	gdt_flush((uint32_t) &gdt_ptr);
	tss_flush();
}


void tss_set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
}

/* GDT and TSS functions */
void init_descriptor_tables()
{
	init_gdt();
}
