#ifndef _YANIX_SYS_UTSNAME_H
#define _YANIX_SYS_UTSNAME_H

#define _UTSNAME_ENTRY_LENGTH 65

struct utsname
{
	char sysname[_UTSNAME_ENTRY_LENGTH];
	char nodename[_UTSNAME_ENTRY_LENGTH];
	char release[_UTSNAME_ENTRY_LENGTH];
	char version[_UTSNAME_ENTRY_LENGTH];
	char machine[_UTSNAME_ENTRY_LENGTH];
};

int uname(struct utsname *buf);

#endif /* _YANIX_SYS_UTSNAME_H */