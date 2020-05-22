#include <stdint.h>
#include <stddef.h>


/*
this library will hold a lot of functions with various purposes

(functions from functions.h, bit.h, some math.h functions)
furthermore: memory management functions etc...
*/

int atoi(const char *str)
{
	int num = 0;

	while (*str)
	{
		if (*str > '9' || *str < '0')
			return num;

		num *= 10;
		num += *str - '0';
		str++;
	}

	return num;
}