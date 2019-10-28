#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void pchar(char c)
{
	write(1, &c, 1);
}

void print_int(unsigned int x)
{
	unsigned int reversed_int = 0;
	unsigned int zeros_before = 0;

	while (x > 0)
	{
		reversed_int = reversed_int * 10 + (x % 10);
		if ((reversed_int == 0) && ((x % 10) == 0))
			zeros_before++;
		x /= 10;
	}

	if (reversed_int == 0) { pchar('0'); }

	while (reversed_int > 0)
	{
		pchar('0' + (reversed_int % 10));
		reversed_int /= 10;
	}

	for (size_t i = 0; i < zeros_before; i++)
	{
		pchar('0');
	}

}

void print(char *msg)
{
	char c;
	size_t i = 0;
	while ((c = msg[i]) != 0){
		pchar(c);
		++i;
	}
}

int main(int argc, char const *argv[])
{
	print("Normal print, home made\n");
	putchar('X');
	return 0;
}
//804835f
