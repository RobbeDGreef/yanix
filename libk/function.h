#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdint.h>

#define UNUSED(x) (void)(x)

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

extern void panic(char *message, char *file, uint32_t line);
extern void panic_assert(char *file, uint32_t line, char *desc);

#endif