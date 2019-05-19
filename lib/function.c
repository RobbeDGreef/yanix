#include <lib/function.h>
#include <drivers/video/videoText.h>

extern void panic(char *message, char *file, uint32_t line)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

    print("PANIC(");
    print(message);
    print(") at ");
    print(file);
    print(":");
    print_int((int)line);
    print("\n");
    // Halt by going into an infinite loop.
    for(;;);
}

extern void panic_assert( char *file, uint32_t line, char *desc)
{
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    print("ASSERTION-FAILED(");
    print(desc);
    print(") at ");
    print(file);
    print(":");
    print_int((int)line);
    print("\n");
    // Halt by going into an infinite loop.
    for(;;);
}