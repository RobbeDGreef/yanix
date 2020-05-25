#include <debug.h>
#include <stdarg.h>

void DEBUGPRINT(char *str)
{
	// if (check_vfs_initialised())
	//	printk(str);
	// else
	serial_write(str);
}

void DEBUGPUT(char c)
{
	serial_put(c);
}

void debug_print(char *str)
{
	DEBUGPRINT(str);
}

void debug_print_hex(unsigned int val)
{
	debug_print(int_to_str(val));
}

void debug_print_m(char *str, ...)
{
	debug_print(str);

	va_list valist;
	va_start(valist, str);

	char *arg = va_arg(valist, char *);

	while (arg != 0)
	{
		debug_print(arg);
		arg = va_arg(valist, char *);
	}

	va_end(valist);
}

void hang()
{
	for (;;)
		asm volatile("hlt;");
}

#pragma GCC diagnostic ignored "-Wframe-address"

inline void __attribute__((always_inline)) dump_stacktrace()
{
	debug_print_m("Stack trace from: ", __func__, "\n", 0);

	/* I would put this in a loop but gcc only wants constant values so whatever
	 * ill write it out */
	debug_print("~ ");
	debug_print_hex((int) get_return_address(0));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(1));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(2));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(3));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(4));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(5));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(6));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(7));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(8));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(9));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(10));
	debug_print("\n");
	debug_print("~ ");
	debug_print_hex((int) get_return_address(11));
	debug_print("\n");
}

void print_stack()
{
	unsigned long stack;
	asm volatile("mov %%esp, %0" : "=r"(stack));
	debug_print("Stack: ");
	debug_print_hex(stack);
	debug_print("\n");
}

void __attribute__((noinline)) print_return_address()
{
	debug_print("Return address: ");
	debug_print_hex((int) get_return_address(1));
	debug_print("\n");
}

static void putchark(char c)
{
	DEBUGPUT(c);
}

static void print(char *str)
{
	DEBUGPRINT(str);
}

/**
 * @brief      Literally just copied the printk function and changed it a bit
 */
void debug_printk(const char *__restrict fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	size_t i = 0;
	char   character;

	while ((character = fmt[i]) != '\0')
	{
		if (character == '%')
		{
			/* escape character */
			if (fmt[i + 1] == '%')
			{
				/* %% means just print a procent sign */
				putchark('%');
			}
			else if (fmt[i + 1] == 'c')
			{
				/* Print a character here */
				char tmp_char = va_arg(args, int);
				putchark(tmp_char);
			}
			else if (fmt[i + 1] == 's')
			{
				/* Print a string here */
				char *tmp_str = va_arg(args, char *);
				print(tmp_str);
			}
			else if (fmt[i + 1] == 'i')
			{
				/* Print a signed integer here (base 10) */
				int tmp_int = va_arg(args, int);

				/* Check if the int is negative */
				int isneg = 0;
				if (tmp_int < 0)
				{
					isneg   = 1;
					tmp_int = -tmp_int;
				}

				/* This piece of code reverses the integer */
				int          reversed_int = 0;
				unsigned int zeros_before = 0;
				while (tmp_int > 0)
				{
					reversed_int = reversed_int * 10 + tmp_int % 10;
					if ((reversed_int == 0) && ((tmp_int % 10) == 0))
						zeros_before++;

					tmp_int /= 10;
				}

				/* If the integer is negative print a '-' sign before it */
				if (isneg)
					putchark('-');

				/* @todo: 0's aren't printed, quick hack */
				if (reversed_int == 0)
				{
					putchark('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					putchark('0' + (reversed_int % 10));
					reversed_int /= 10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					putchark('0');
				}
			}
			else if (fmt[i + 1] == 'u')
			{
				/* Print a unsigned integer here (base 10) */
				unsigned int tmp_int = va_arg(args, unsigned int);

				/* This piece of code reverses the integer */
				unsigned int reversed_int = 0;
				unsigned int zeros_before = 0;
				while (tmp_int > 0)
				{
					reversed_int = reversed_int * 10 + tmp_int % 10;
					if ((reversed_int == 0) && ((tmp_int % 10) == 0))
						zeros_before++;
					tmp_int /= 10;
				}

				/* @todo: 0's aren't printed, quick hack */
				if (reversed_int == 0)
				{
					putchark('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					putchark('0' + (reversed_int % 10));
					reversed_int /= 10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					putchark('0');
				}
			}
			else if (fmt[i + 1] == 'x')
			{
				/* Print hexadecimal numbers */
				unsigned int tmp_int = va_arg(args, unsigned int);

				/* This piece of code reverses the integer */
				unsigned int reversed_int = 0;
				unsigned int zeros_before = 0;
				while (tmp_int > 0)
				{
					reversed_int = reversed_int * 0x10 + tmp_int % 0x10;
					if ((reversed_int == 0) && ((tmp_int % 0x10) == 0))
						zeros_before++;
					tmp_int /= 0x10;
				}

				/* @todo: 0's aren't printed, quick hack */
				if (reversed_int == 0)
				{
					putchark('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					if ((reversed_int % 0x10) >= 0xA)
					{
						putchark('A' + ((reversed_int % 0x10) - 10));
					}
					else
					{
						putchark('0' + (reversed_int % 0x10));
					}

					reversed_int /= 0x10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					putchark('0');
				}
			}
			else if (fmt[i + 3] == 'x')
			{
				/* Note: this is definitely not a perfect / bugproof system but
				 * it works */
				/* Print hexadecimal with specific string length (given by
				 * fmt[i+1], fmt[i+2]) */
				unsigned int tmp_int = va_arg(args, unsigned int);
				int wanted_bits = (fmt[i + 1] - '0') * 10 + fmt[i + 2] - '0';

				/* Calculate the size of the integer */
				int          intsize = 0;
				unsigned int tmp     = tmp_int;
				while (1)
				{
					if (tmp == 0)
					{
						break;
					}
					tmp /= 0x10;
					intsize++;
				}

				/* This piece of code reverses the integer */
				unsigned int reversed_int = 0;
				unsigned int zeros_before = 0;
				while (tmp_int > 0)
				{
					reversed_int = reversed_int * 0x10 + tmp_int % 0x10;
					if ((reversed_int == 0) && ((tmp_int % 0x10) == 0))
						zeros_before++;
					tmp_int /= 0x10;
				}

				/* Print amount of starting zero's */
				for (int i = 0; i < (wanted_bits - intsize); i++)
				{
					putchark('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					if ((reversed_int % 0x10) >= 0xA)
					{
						putchark('A' + ((reversed_int % 0x10) - 10));
					}
					else
					{
						putchark('0' + (reversed_int % 0x10));
					}

					reversed_int /= 0x10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					putchark('0');
				}
				i += 2;
			}

			/* Skip over next character */
			i++;
		}
		else
		{
			/* A regular character to print */
			putchark(fmt[i]);
		}

		i++;
	}

	va_end(args);
}

int random_complex_calc(int x)
{
	float y = -1.0;

	for (int i = 0; i < x; i++)
	{
		y += (i + 1) / 3;
	}

	return y;
}
