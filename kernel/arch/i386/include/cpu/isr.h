#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include <cpu/cpu.h>


void isr_install();
void isr_handler(registers_t *r);

#endif

