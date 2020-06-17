#ifndef _NET_SOCKET_UNIX
#define _NET_SOCKET_UNIX

#include <yanix/sys/un.h>

struct socket_conn;
struct socket_conn *unix_socket(int type, int protocol);

#endif /* _NET_SOCKET_UNIX */