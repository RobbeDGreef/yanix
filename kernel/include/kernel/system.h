#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <yanix/sys/sysinfo.h>

struct kern_sysinfo
{
	char *hostname;
	char *sysname;
	char *release;
	char *version;
	char *machine;

	unsigned long totalram;
	unsigned long totalswap;
};

struct us_sysinfo
{
	long uptime;
	unsigned long loads[3];
	unsigned long totalram;
	unsigned long freeram;
	unsigned long sharedram;
	unsigned long bufferram;
	unsigned long totalswap;
	unsigned long freeswap;
	unsigned short procs;
	char _pad[22];
};

extern struct kern_sysinfo g_system;

int init_sysinfo();

#endif /* _SYSTEM_H */