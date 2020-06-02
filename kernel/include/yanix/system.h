#ifndef _SYSTEM_H
#define _SYSTEM_H

struct kern_sysinfo
{
	char *hostname;
	char *sysname;
	char *release;
	char *version;
	char *machine;
};

extern struct kern_sysinfo g_system;

int init_sysinfo();

#endif /* _SYSTEM_H */