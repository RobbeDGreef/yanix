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
#include <cpuid.h> /* GCC header file */
#include <kernel.h>
#include <libk/string.h>
#include <mm/heap.h>

void cpuid_cmd(struct cpuid_reg *regs, int cmd);

/**
 * @brief      Initializes the architecture
 */
void arch_init()
{
	/* Init the GDT */
	init_descriptor_tables();
	isr_install();
}

void arch_cpu_string()
{
	struct cpuid_reg regs;
	cpuid_cmd(&regs, 0);

	char *str = kmalloc(13);

	uint32_t *buf = (uint32_t *) str;
	buf[0]        = regs.ebx;
	buf[1]        = regs.edx;
	buf[2]        = regs.ecx;

	str[12] = '\0';
	return str;
}