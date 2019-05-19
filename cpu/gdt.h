#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#if 0 // not sure if still needed or not
struct gdt_entry_s{
   uint32_t limit_low: 16;
   uint32_t base_low:  24;

   uint32_t accessed:                 1;
   uint32_t read_write:               1; //readable for code, writable for data
   uint32_t conforming_expand_down:   1; //conforming for code, expand down for data
   uint32_t code:                     1; //1 for code, 0 for data
   uint32_t always_1:                 1; //should be 1 for everything but TSS and LDT
   uint32_t DPL:                      2; //priviledge level
   uint32_t present:                  1;

   uint32_t limit_high:   4;
   uint32_t available:    1;
   uint32_t always_0:     1; //should always be 0
   uint32_t big:          1; //32bit opcodes for code, uint32_t stack for data
   uint32_t gran:         1; //1 to use 4k page addressing, 0 for byte addressing
   uint32_t base_high:    8;

} __attribute__((packed));

struct gdt_ptr_s{
   uint16_t    limit;   // the upper 16 bits of all selector limits
   uint32_t    base;    // the address of the first gdt_entry_t struct
}__attribute__((packed));

typedef struct gdt_entry_s gdt_entry_t;

extern gdt_entry_t* gdtlist[];
void init_kernel_gdt(uint32_t code_seg, uint32_t data_seg);

#endif

struct gdt_entry_s {
   uint16_t       limit_low;
   uint16_t       base_low;
   uint8_t        base_middle;
   uint8_t        access;
   uint8_t        granularity;
   uint8_t        base_high;
}__attribute__((packed));

typedef struct gdt_entry_s gdt_entry_t;

struct gdt_ptr_s {
   uint16_t       limit;  
   uint32_t       base;    // address of fist gdt_entry_t struct
} __attribute__((packed));

typedef struct gdt_ptr_s gdt_ptr_t;

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

void init_descriptor_tables();
void tss_set_kernel_stack(uint32_t stack);

#endif