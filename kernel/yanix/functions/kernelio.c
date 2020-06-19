#include <debug.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <proc/tasking.h>
#include <stdarg.h>
#include <kernel/tty_dev.h>

unsigned int g_kernel_log_level;

/**
 * @brief      Puts 1 character to the screen
 *
 * @param[in]  character  The character
 */
static void putchark(char character)
{
	vfs_write_fd(1, &character, 1);
}

char wbuffer[128];
int  iter = 0;

static void flushbuf()
{
	vfs_write_fd(1, wbuffer, iter);
	iter = 0;
}

static void writebuf(char c)
{
	if (iter == 127)
		flushbuf();

	wbuffer[iter++] = c;
}

static size_t print(const char *txt, size_t len)
{
	return vfs_write_fd(1, txt, len);
}

/**
 * @brief      A general print function for the kernel with logging capabilities
 *
 * @param[in]  fmt        The format string
 * @param[in]  ... 		  The format string arguments
 *
 * @return     Amount of character written to the screen
 */
int printk(const char *__restrict fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	/* Current log level of kernel */
	int loglevel = g_kernel_log_level;

	size_t i                 = 0;
	size_t written_character = 0;
	char   character;

	/* In case a log level is specified follow it */
	/* This checks if the loglevel is specified */
	if (fmt[0] == '<' && (fmt[1] <= '7' && fmt[i] >= '0') && fmt[2] == '>')
	{
		/* All printable number characters go up from character '0' so just a
		 * little calculation here */
		loglevel = (int) fmt[1] - '0';
		i        = 3;
	}

	/* @todo: loglevel should be used properly in this code*/

	if (loglevel == 7)
	{
		// tty_set_color(TTY_YELLOW, -1);

		/* Debug loglevel */
		printk("\x1b[33;1m[ DEBUG ] ");
	}
	else if (loglevel == 6)
	{
		printk("[ INFO ]  ");
	}
	else if (loglevel == 4)
	{
		tty_set_color(TTY_ORANGE, -1);
		printk("[WARNING] ");
	}
	else if (loglevel == 5)
	{
		tty_set_color(TTY_LIGHT_BLUE, -1);
		printk("[NOTICE] ");
	}

	while ((character = fmt[i]) != '\0')
	{
		if (character == '%')
		{
			/* escape character */
			if (fmt[i + 1] == '%')
			{
				/* %% means just print a procent sign */
				writebuf('%');
				written_character++;
			}
			else if (fmt[i + 1] == 'c')
			{
				/* Print a character here */
				char tmp_char = va_arg(args, int);
				writebuf(tmp_char);
				written_character++;
			}
			else if (fmt[i + 1] == 's')
			{
				/* Print a string here */
				const char *tmp_str = va_arg(args, const char *);
				written_character += print(tmp_str, strlen(tmp_str));
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
					writebuf('-');

				/* @todo: 0's aren't printed, quick hack */
				if (reversed_int == 0)
				{
					writebuf('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					writebuf('0' + (reversed_int % 10));
					reversed_int /= 10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					writebuf('0');
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
					writebuf('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					writebuf('0' + (reversed_int % 10));
					reversed_int /= 10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					writebuf('0');
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
					writebuf('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					if ((reversed_int % 0x10) >= 0xA)
					{
						writebuf('A' + ((reversed_int % 0x10) - 10));
					}
					else
					{
						writebuf('0' + (reversed_int % 0x10));
					}

					reversed_int /= 0x10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					writebuf('0');
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
					writebuf('0');
				}

				/* This piece of code prints every character in the integer */
				while (reversed_int > 0)
				{
					if ((reversed_int % 0x10) >= 0xA)
					{
						writebuf('A' + ((reversed_int % 0x10) - 10));
					}
					else
					{
						writebuf('0' + (reversed_int % 0x10));
					}

					reversed_int /= 0x10;
				}

				/* If there were any zero's before the integer print them now */
				for (size_t i = 0; i < zeros_before; i++)
				{
					writebuf('0');
				}
				i += 2;
			}

			/* Skip over next character */
			i++;
		}
		else
		{
			/* A regular character to print */
			writebuf(fmt[i]);
			written_character++;
		}

		i++;
	}

	flushbuf();

	if (loglevel)
	{
		printk("\x1b[0m");
		tty_reset_color();
	}
	va_end(args);
	return written_character;
}

/**
 * @brief      Clears the screen
 */
void clear_screenk()
{
	tty_clear_buf(tty_get_device(get_current_task()->tty));
}

void printk_hd(void *ptr, size_t size)
{
	for (size_t i = 0; i <= size; i += 0x10)
	{
		unsigned char *tmp = (unsigned char *) (((unsigned int) ptr) + i);
		printk("%08x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x "
		       "%02x %02x %02x %02x %02x %02x\n",
		       (unsigned int) ptr + i, (unsigned char) *(tmp),
		       (unsigned char) *(tmp + 1), (unsigned char) *(tmp + 2),
		       (unsigned char) *(tmp + 3), (unsigned char) *(tmp + 4),
		       (unsigned char) *(tmp + 5), (unsigned char) *(tmp + 6),
		       (unsigned char) *(tmp + 7), (unsigned char) *(tmp + 8),
		       (unsigned char) *(tmp + 9), (unsigned char) *(tmp + 10),
		       (unsigned char) *(tmp + 11), (unsigned char) *(tmp + 12),
		       (unsigned char) *(tmp + 13), (unsigned char) *(tmp + 14),
		       (unsigned char) *(tmp + 15));
	}
}
