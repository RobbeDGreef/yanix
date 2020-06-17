#include <net/socket.h>
#include <net/socket/unix.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <errno.h>
#include <libk/string.h>
#include <fs/pipe.h>
#include <debug.h>

struct socket *unix_socket_setup(int type, int protocol);

struct socket *socket_from_addr(const struct sockaddr *_addr)
{
	const struct sockaddr_un *addr = (struct sockaddr_un *) _addr;

	vfs_node_t *snode = vfs_find_path(addr->sun_path);
	if (snode == NULL)
		return NULL;

	struct socket *soc = (struct socket *) snode->offset;

	return soc;
}

int unix_socket_bind(struct socket_conn *soc_con, const struct sockaddr *_addr,
                     socklen_t addrlen)
{
	const struct sockaddr_un *addr = (struct sockaddr_un *) _addr;

	struct socket *tobind = socket_from_addr(_addr);
	if (tobind == NULL)
		return -1;

	soc_con->soc = tobind;
	return 0;
}

int unix_socket_listen(struct socket_conn *soc, int backlog)
{
	if (soc->soc->server_lock)
	{
		errno = EADDRINUSE;
		return -1;
	}

	soc->soc->server_backlog = backlog;
	return 0;
}

int unix_socket_connect(struct socket_conn *soc, const struct sockaddr *addr,
                        socklen_t addrlen)
{
	if (!soc->soc->server_lock)
	{
		errno = ECONNREFUSED;
		return -1;
	}

	soc->client = 1;
	return unix_socket_bind(soc, addr, addrlen);
}

int unix_socket_send(struct socket_conn *conn, const void *buf, size_t size,
                     int flags)
{
	if (!conn || !conn->soc)
		return -1;

	struct pipe *pipe;
	if (conn->client)
		pipe = conn->client_pipe;
	else
		pipe = conn->server_pipe;

	return pipe_write_raw(pipe, buf, size);
}

int unix_socket_recv(struct socket_conn *conn, void *buf, size_t size,
                     int flags)
{
	if (!conn || !conn->soc)
		return -1;

	struct pipe *pipe;
	if (conn->client)
		pipe = conn->client_pipe;
	else
		pipe = conn->server_pipe;

	return pipe_read_raw(pipe, buf, size);
}

int unix_socket_tryconnect(struct socket_conn *conn, )
{
	// Ask server nicely if it is possible to connect
	// -> try to unblock server and see if it was blocked or not (if it wasnt
	// fail) block for response ? but what abt fucking threads then UGH
}

int unix_socket_accept(struct socket_conn *conn, struct sockaddr *addr,
                       socklen_t *addrlen)
{
	/* Block until connect signal sent by unix_socket_tryconnect */
	soc->soc->server_lock = getpid();

	task_block(getpid());

	// Unqueue latest socket connection request
	// send the socket success or failure
	// somehow both connections need enough data to be able to talk
	// this means transfering server_pipe and client_pipe or smthn
}

struct socket *unix_socket_setup(int type, int protocol)
{
	struct socket *soc = kmalloc(sizeof(struct socket));
	memset(soc, 0, sizeof(struct socket));

	soc->type     = type;
	soc->protocol = protocol;

	soc->bind    = unix_socket_bind;
	soc->listen  = unix_socket_listen;
	soc->connect = unix_socket_connect;
	soc->recv    = unix_socket_recv;
	soc->send    = unix_socket_send;
	soc->accept  = unix_socket_accept;

	return soc;
}

struct socket *unix_socket_create(struct socket_conn *conn,
                                  struct sockaddr *_addr, socklen_t addrlen)
{
	struct sockaddr_un *addr = (struct sockaddr_un *) _addr;
	struct socket *     soc  = unix_socket_setup(conn->type, conn->protocol);
	if (soc == NULL)
		return NULL;

	char *path = vfs_fullpath(addr->sun_path);
	if (vfs_sock_create(path, soc))
		return NULL;

	return soc;
}

struct socket_conn *unix_socket(int type, int protocol)
{
	struct socket_conn *conn = kmalloc(sizeof(struct socket_conn));
	memset(conn, 0, sizeof(struct socket_conn));

	conn->type          = type;
	conn->protocol      = protocol;
	conn->create_socket = unix_socket_create;

	return conn;
}
