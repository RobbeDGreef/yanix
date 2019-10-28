#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/* IRQ values */
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47


/**
 * @brief      The GDT and TSS functions
 * 
 * @file 	   cpu/gdt.c
 */
void init_descriptor_tables();

/* Struct which aggregates many registers */
typedef struct {
    uint32_t ds; /* Data segment selector */
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax; /* Pushed by pusha. */
    uint32_t int_no, err_code; /* Interrupt number and error code (if applicable) */
    uint32_t eip, cs, eflags, esp, ss; /* Pushed by the processor automatically */
}__attribute__((packed)) registers_t ;

typedef void (*isr_callback_t)(registers_t*);
void arch_register_interrupt_handler(uint8_t n, isr_callback_t handler);

/**
 * @brief      Initializes the architecture
 */
void arch_init();



#endif