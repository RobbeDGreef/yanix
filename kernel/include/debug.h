#ifndef DEBUG_H
#define DEBUG_H

/**
 * Just a few debugging functions, a lot of functions from
 * different systems inside the kernel are all collected here
 */

#include <core/timer.h>
#include <cpu/cpu.h>
#include <drivers/serial.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <libk/string.h>
#include <mm/linkedlist.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel/kfunctions.h>

#define DEFAULT_STACKTRACE_DEPTH 8

#define DEBUG()        (asm volatile("int $0x3"))
#define NOTICE_POINT() asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;")
void printk_hd(void *ptr, size_t size);

#define get_return_address __builtin_return_address

void debug_handler(registers_t *regs);
void debug_handler_task();
void dump_stacktrace();
void debug_print_m(char *str, ...);
void debug_print(char *str);
void debug_print_hex(unsigned int val);
void debug_printk(const char *__restrict fmt, ...);

void hang();
void print_return_address();
void print_stack();

int   check_vfs_initialised();
void  debug_print_phys_frame(offset_t virt_addr, size_t size,
                             page_directory_t *dir);
int   random_complex_calc(int x);
reg_t getstack();
int   debug_is_heapblock_free(void *);

#endif