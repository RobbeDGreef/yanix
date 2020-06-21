#include <cpu/cpu.h>
#include <cpu/cpuid.h>

static int has_apic()
{
	struct cpuid_reg regs;
	cpuid_cmd(&regs, 1);
	return (regs.edx & CPUID_FEAT_EDX_APIC) ? 0 : -1;
}

static uint32_t apic_read(uint32_t *base, uint32_t reg, uint32_t value)
{
	/* Write to reg BASE+0x0 and read from BASE+0x10 */
	base[0] = reg & 0xFF;
	return base[4];
}

static void apic_write(uint32_t *base, uint32_t reg, uint32_t value)
{
	/* Write to reg BASE+0x0 and BASE+0x10 */
	base[0] = reg & 0xFF;
	base[4] = value;
}

int init_apic()
{
	/* Currently not supported */
	return -1;

	if (has_apic())
		return -1;

	return 0;
}

void apic_send_eio(int inter)
{
}