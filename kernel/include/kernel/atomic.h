#ifndef _KERNEL_ATOMIC_H
#define _KERNEL_ATOMIC_H

#include <cpu/atomic.h>

#define atomic_test_and_set(mem, bit) arch_atomic_test_and_set(mem, bit)

#endif /* _KERNEL_ATOMIC_H */