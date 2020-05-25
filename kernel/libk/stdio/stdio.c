#include <fs/dirent.h>
#include <fs/vfs.h>
#include <proc/syscall.h>
#include <stdint.h>

// @todo: this should use system calls
#include <debug.h>

int readline(char *data, int dataend, char *buf, int size)
{
	int i;
	for (i = 0; i < size - 1; i++)
	{
		if (i == dataend)
			return -1;

		buf[i] = data[i];

		if (data[i] == '\n')
		{
			i++;
			break;
		}
	}

	buf[i] = '\0';
	return i;
}