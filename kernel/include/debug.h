#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdint.h>
#include <kernel.h>
#include <drivers/serial.h>

#define DEBUG() {asm volatile ("int $0x3");}

void printk_hd(void *ptr, size_t size);

#endif