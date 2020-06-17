#ifndef _YANIX_SYS_UN_H
#define _YANIX_SYS_UN_H

#define UN_SOC_PATH_LEN 108

struct sockaddr_un
{
	int  sun_family;
	char sun_path[UN_SOC_PATH_LEN];
};

#endif /* _YANIX_SYS_UN_H */