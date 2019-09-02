#include <cpu/gdt.h>
#include <libk/string/string.h>
#include <stdint.h>

#if 0 // not sure if strill needed
gdt_entry_t *gdtlist[5];

extern void flush_tss(void); // see flush_tss.asm

void init_kernel_gdt(uint32_t code_seg, uint32_t data_seg){
	// the ring 0 gdt's
	gdtlist[0] = (gdt_entry_t*) code_seg-8; 	// this is kind of a hack but it works because that's how we set up the gdt in the bootloader
	gdtlist[1] = (gdt_entry_t*) code_seg;
	gdtlist[2] = (gdt_entry_t*) data_seg;
}

/*

void init_user_gdt(){
	gdt_entry_t *code = (gdt_entry_t*) &gdtlist[3];
	gdt_entry_t *data = (gdt_entry_t*) &gdtlist[4];
	
	code->limit_low  = 0xFFFF;
	code->base_low   = 0;
	code->accessed   = 0;
	code->read_write = 1;
	code->conforming_expand_down = 0;
	code->code 		 = 1;
	code->always_1 	 = 1;
	code->DPL		 = 3; //set ring level
	code->present	 = 1;
	code->limit_high = 0xF;
	code->available	 = 1;
	code->always_0 	 = 0;
	code->big 		 = 1;
	code->gran 		 = 1;
	code->base_high  = 0;
	
	*data = *code;
	data->code = 0; //signal it's not code; so it's data.

	install_tss(&gdtlist[5]);
	flush_tss();
}
*/

#endif

// asm function to flush gdt and tss
extern void gdt_flush(uint32_t);
extern void tss_flush();

static void init_gdt();
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);

gdt_entry_t 	gdt_entries[6];
gdt_ptr_t 		gdt_ptr;

tss_entry_t 	tss_entry;

void init_descriptor_tables()
{
	init_gdt();
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
}

void tss_set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
}

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