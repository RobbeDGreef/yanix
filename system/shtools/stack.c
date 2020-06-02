#include <stdint.h>
#include <stdio.h>

int main()
{
	uint32_t esp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	printf("current stack: %x\n", esp);
}