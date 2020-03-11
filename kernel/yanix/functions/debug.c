#include <debug.h>
#include <stdarg.h>


void DEBUGPRINT(char *str)
{
	if (check_vfs_initialised())
		printk(str);
	else
		serial_write(str);
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

	char *arg = va_arg(valist, char*);

	while (arg != 0)
	{
		debug_print(arg);
		arg = va_arg(valist, char*);
	}

	va_end(valist);
}

void hang()
{
	for(;;)
		asm volatile ("hlt;");
}


#pragma GCC diagnostic ignored "-Wframe-address"

inline void __attribute__((always_inline)) dump_stacktrace()
{
	debug_print_m("Stack trace from: ", __func__, "\n", 0); 
	
	/* I would put this in a loop but gcc only wants constant values so whatever ill write it out */	
	debug_print("~ "); debug_print_hex((int) get_return_address(0)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(1)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(2)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(3)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(4)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(5)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(6)); debug_print("\n");
	debug_print("~ "); debug_print_hex((int) get_return_address(7)); debug_print("\n");
}

