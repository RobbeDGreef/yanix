#include <stdio.h>

int main()
{
	printf("Going to call privileged instruction\n");
	asm volatile("hlt;");
}