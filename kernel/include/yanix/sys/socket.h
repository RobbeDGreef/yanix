#ifndef _YANIX_SYS_SOCKET_H
#define _YANIX_SYS_SOCKET_H

#include <sys/types.h>

#define AF_UNIX     1
#define SOCK_STREAM 2
#define SOCK_DGRAM  3

typedef int sa_family_t;
typedef int socklen_t;

struct sockaddr
{
	sa_family_t sa_family;
	char        sa_data[4];
};

int socket(int domain, int type, int protocol);
int bind(int soc, const struct sockaddr *addr, socklen_t addrlen);
int listen(int soc, int backlog);
int accept(int soc, struct sockaddr *addr, socklen_t *addrlen);
int send(int soc, const void *buf, size_t len, int flags);
int recv(int soc, void *buf, size_t len, int flags);
int connect(int soc, const struct sockaddr *, socklen_t addrlen);

#endif /* _YANIX_SYS_SOCKET_H */