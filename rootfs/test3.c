#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	printf("Reading from the stdin");
	char c;
	while (read(0, &c, 1) > 0)
	{
		printf("char: %c\n", c);
	}
	printf("End");
}
