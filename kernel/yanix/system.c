#include <config.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <yanix/system.h>

struct kern_sysinfo g_system;

int init_sysinfo()
{
	char         buf[256];
	struct file *fp = vfs_open("/etc/hostname", 0, 0);
	vfs_read(fp, buf, 256);
	vfs_close(fp);

	g_system.hostname = strdup(buf);
	g_system.sysname  = OS_NAME;
	g_system.release  = OS_RELEASE;
	g_system.version  = OS_VERSION;
	g_system.machine  = HW_IDENT;

	return 0;
}