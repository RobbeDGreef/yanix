#ifndef DEBUG_H
#define DEBUG_H

#include <stddef.h>
#include <stdint.h>
#include <kernel.h>
#include <drivers/serial.h>
#include <yanix/kfunctions.h>
#include <libk/string.h>
#include <fs/vfs.h>
#include <drivers/serial.h>
#include <cpu/cpu.h>

#define DEFAULT_STACKTRACE_DEPTH	8

#define DEBUG() (asm volatile ("int $0x3"))
void printk_hd(void *ptr, size_t size);

#define get_return_address __builtin_return_address

void debug_handler(registers_t *regs);
void debug_handler_task();
void dump_stacktrace();
void debug_print_m(char *str, ...);
void debug_print(char *str);
void debug_print_hex(unsigned int val);
void debug_printk(const char* __restrict fmt, ...);

void hang();
void print_return_address();
void print_stack();

int check_vfs_initialised();
int random_complex_calc(int x);


#endif
