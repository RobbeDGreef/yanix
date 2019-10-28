/**
 * @defgroup   ARCH cpu
 *
 * @brief      This file implements the main cpu architecture functions.
 *
 * @author     Robbe De Greef
 * @date       2019
 */
#include <cpu/gdt.h>
#include <cpu/isr.h>

/**
 * @brief      Initializes the architecture
 */
void arch_init()
{
	/* Init the GDT */
	init_descriptor_tables();
	isr_install();

	/* Enabling interrupts */
	asm volatile ("sti");
}