#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define KERNEL_CS 	0x08
#define IDT_ENTRIES 256

/**
 * Structure for the idt entry
 */
typedef struct {
        uint16_t 	low_offset;
        uint16_t	sel;			// kernel segment selector
        uint8_t 	zero;
        uint8_t 	flags;
        uint16_t 	high_offset;
}__attribute__((packed)) idt_gate_t;

/**
 * Structure for the idt pointer
 */
typedef struct{
        uint16_t limit;
        uint32_t base;
}__attribute__((packed)) idt_register_t;

extern idt_gate_t 	  idt[IDT_ENTRIES];
extern idt_register_t idt_reg;

void set_idt_gate(int32_t n, uint32_t handler);
void set_idt();

#endif
