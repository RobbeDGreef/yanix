#include <yanix/ioctl.h>
#include <debug.h>
#include <kernel/tty_dev.h>

int termios_cmd(int request, char *argv)
{
	debug_printk("received termios cmd %i\n", request);
	switch (request)
	{
	case TC_DISABLE:
		tty_disable();
		return 0;

	case TC_ENABLE:
		tty_enable();
		return 0;
	}

	return -1;
}