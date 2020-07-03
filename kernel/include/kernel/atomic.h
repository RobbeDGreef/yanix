#ifndef _KERNEL_ATOMIC_H
#define _KERNEL_ATOMIC_H

#include <cpu/atomic.h>


#define atomic_test_and_set(mem, bit) arch_atomic_test_and_set(mem, bit)
#define atomic_compare_exchange(mem, bitunset, bitset) \
	arch_atomic_compare_exchange(mem, bitunset, bitset)

#define atomic_store(mem, bit) arch_atomic_store(mem, bit)

#endif /* _KERNEL_ATOMIC_H */