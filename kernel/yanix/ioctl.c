#include <yanix/sys/ioctl.h>
#include <fs/filedescriptor.h>
#include <debug.h>
#include <stdarg.h>

int _ioctl(int fd, int request, char *args)
{
	debug_printk("ioctl\n");
	struct file_descriptor *fp = get_filedescriptor(fd);

	if (!fp || !fp->node || !fp->node->cmd)
		return -1;

	return fp->node->cmd(request, args);
}

int ioctl(int fd, int request, ...)
{
	va_list vl;
	va_start(vl, request);
	int ret = _ioctl(fd, request, va_arg(vl, char *));
	va_end(vl);
	return ret;
}
