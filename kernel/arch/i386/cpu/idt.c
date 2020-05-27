#include <cpu/idt.h>
#include <stdint.h>

#define low16(address)  (uint16_t)((address) &0xFFFF)
#define high16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

idt_gate_t     idt[IDT_ENTRIES];
idt_register_t idt_reg;

/**
 * @brief      Sets an idt gate.
 *
 * @param[in]  n        number of the gate
 * @param[in]  handler  The handler
 */
void set_idt_gate(int32_t n, uint32_t handler)
{
	idt[n].low_offset = low16(handler);
	idt[n].sel        = KERNEL_CS;
	idt[n].zero       = 0;
	/* the | 60 needs to be set in order for usermode to work */
	idt[n].flags       = 0x8e | 0x60;
	idt[n].high_offset = high16(handler);
}

void set_task_gate(int32_t n, uint32_t handler)
{
	/* I am unable to figure out how these work / should be set up so yeah */
	uint16_t tss_seg_sel = 0x28;
	idt[n].low_offset    = 0;
	idt[n].sel           = tss_seg_sel;
	idt[n].zero          = 0;
	idt[n].flags         = 0x85;
	idt[n].high_offset   = 0;
}

/**
 * @brief      Sets the idt.
 */
void set_idt()
{
	idt_reg.base  = (uint32_t) &idt;
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
	asm volatile("lidtl (%0)" : : "r"(&idt_reg));
}