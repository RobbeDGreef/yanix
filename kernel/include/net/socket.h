#ifndef _NET_SOCKET
#define _NET_SOCKET

#include <net/socket/unix.h>
#include <sys/types.h>
#include <fs/vfs_node.h>
#include <yanix/sys/socket.h>
#include <kernel/ds/queue.h>

struct socket;
struct socket_conn;

typedef int (*sb_fptr)(struct socket_conn *, const struct sockaddr *,
                       socklen_t);
typedef int (*sl_fptr)(struct socket_conn *, int);
typedef int (*ss_fptr)(struct socket_conn *, const void *, size_t, int);
typedef int (*sr_fptr)(struct socket_conn *, void *, size_t, int);
typedef int (*sc_fptr)(struct socket_conn *, const struct sockaddr *,
                       socklen_t);

typedef struct socket *(*cs_fptr)(struct socket_conn *, struct sockaddr *addr,
                                  socklen_t addrlength);

typedef struct socket_conn *(*sa_fptr)(struct socket_conn *, struct sockaddr *,
                                       socklen_t *);

struct accept_conn
{
	int          lock;
	struct pipe *client_pipe;
	struct pipe *server_pipe;
};

struct socket_conn
{
	int            client;
	struct socket *soc;

	int type;
	int protocol;

	cs_fptr create_socket;

	struct pipe *send_pipe;
	struct pipe *recv_pipe;
};

struct socket
{
	int protocol;
	int type;

	int           socketfd;
	int           server_lock;
	int           server_backlog;
	struct queue *accept_queue;

	struct sockaddr *addr;
	sa_fptr          accept;
	sb_fptr          bind;
	ss_fptr          send;
	sl_fptr          listen;
	sr_fptr          recv;
	sc_fptr          connect;

	void *fam_specific; /* Family specific data goes here */
};

int socket(int domain, int type, int protocol);
int sock_bind(int soc, const struct sockaddr *addr, socklen_t addrlen);
int sock_listen(int soc, int backlog);
int sock_accept(int soc, struct sockaddr *addr, socklen_t *addrlen);
int sock_send(int soc, const void *buf, size_t len, int flags);
int sock_recv(int soc, void *buf, size_t len, int flags);
int sock_connect(int soc, const struct sockaddr *, socklen_t addrlen);
int sock_close(vfs_node_t *node);

#endif /* _NET_SOCKET */