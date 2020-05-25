#ifndef SYSTEM_H
#define SYSTEM_H

#define sti() asm("sti" ::)
#define cli() asm("cli" ::)
#define nop() asm("asm" ::)

extern void arch_idle_cpu(void);

#endif
