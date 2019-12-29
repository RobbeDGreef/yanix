#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <fs/vfs_node.h>
#include <libk/string.h>
#include <kernel.h>
#include <mm/heap.h>

unsigned char *stdinbuffer;
unsigned int buffersize;
unsigned int buffercount;
unsigned int stdinblocked;

ssize_t tty_stdinwrite(vfs_node_t *node, uint32_t offset, const void *buffer, size_t size)
{
	(void) (node);
	(void) (offset);

	if ((buffercount + size) < buffersize)
	{
		memcpy(&stdinbuffer[buffercount], buffer, size);
		buffercount += size;
	}
	else
	{
		printk(KERN_WARNING "TTY stdin buffer full, todo: write tty buffer to disk if it becommes to big");
		return 0;
	}

	return size;
}

/**
 * @brief      Reads from the stdin
 *
 * @param      node    The node
 * @param[in]  offset  The offset
 * @param      buffer  The buffer
 * @param[in]  size    The size
 *
 * @return     Amount of bytes writen
 */
ssize_t tty_stdinread(vfs_node_t *node, uint32_t offset, void *buffer, size_t size)
{
	/* @todo: Create an actual buffered stdin system instead of weirdly emulating it, also our output is reversed */

	printk("Reading ...\n");

	(void) (node);
	(void) (offset);

	if (stdinblocked)
	{
		/* We need to unblock interrupts to receive keystrokes */
		asm volatile ("sti;");

		/* We need to block the read until a newline is read because this is the 'activating' character */
		while (stdinbuffer[buffercount-1] != '\n');
		stdinblocked = 0;

		asm volatile ("cli;");
	}

	unsigned char *buf = buffer;

	for (unsigned int i = 0; i < size; i++)
	{
		if (buffercount == 0)
		{
			stdinblocked = 1;
			return i;
		}

		buf[i] = stdinbuffer[0];
		buffercount--;

	}
	stdinblocked = 1;
	return size;
}

/**
 * @brief      Initializes the tty stdin.
 */
void init_tty_stdin()
{
	buffersize  = 1024;
	stdinbuffer = (unsigned char*) kmalloc(buffersize);
	buffercount = 0;
	stdinblocked = 1;
}