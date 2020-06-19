#ifndef ARCH_I386_INTERRUPTS_H
#define ARCH_I386_INTERRUPTS_H

void enable_interrupts();
void disable_interrupts();
void end_of_interrupt();
int  are_interrupts_enabled();

#endif /* ARCH_I386_INTERRUPTS_H */
