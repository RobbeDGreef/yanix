#ifndef _ARCH_I386_CPU_ATOMIC_H
#define _ARCH_I386_CPU_ATOMIC_H

int arch_atomic_test_and_set(void *mem, int bitset);
int arch_atomic_compare_exchange(void *mem, int bitunset, int bitset);
int arch_atomic_store(void *mem, int bitset);

#endif /* _ARCH_I386_CPU_ATOMIC_H */