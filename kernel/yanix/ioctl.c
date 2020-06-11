#include <yanix/ioctl.h>
#include <fs/filedescriptor.h>
#include <debug.h>

int ioctl(int fd, int request, char *args)
{
	debug_printk("ioctl\n");
	struct file_descriptor *fp = get_filedescriptor(fd);

	if (!fp || !fp->node || !fp->node->cmd)
		return -1;

	return fp->node->cmd(request, args);
}