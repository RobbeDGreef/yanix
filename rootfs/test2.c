#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main()
{
	printf("Hello world\n");
	void *x = malloc(500);
	printf("Location of bytes: %x\n", x);
	return 0;
}
