#ifndef ISR_H
#define ISR_H

#include <cpu/cpu.h>
#include <stdint.h>

void isr_install();
void isr_handler(registers_t *r);

#endif
