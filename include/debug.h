#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdint.h>



int printk(const char *, ...);

void printk_hd(void *ptr, size_t size);
void printk_hd(void *ptr, size_t size);

#endif