#ifndef _YANIX_SYS_SYSINFO_H
#define _YANIX_SYS_SYSINFO_H

struct sysinfo
{
	long           uptime;
	unsigned long  loads[3];
	unsigned long  totalram;
	unsigned long  freeram;
	unsigned long  sharedram;
	unsigned long  bufferram;
	unsigned long  totalswap;
	unsigned long  freeswap;
	unsigned short procs;
	char           _pad[22];
};

#endif /* _YANIX_SYS_SYSINFO_H */
