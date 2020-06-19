#include <net/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <fs/filedescriptor.h>
#include <fs/vfs.h>

#include <debug.h>
int socket(int domain, int type, int protocol)
{
	struct socket_conn *soc = NULL;
	switch (domain)
	{
	case AF_UNIX:
		soc = unix_socket(type, protocol);
		break;
	default:
		errno = EAFNOSUPPORT;
	}

	if (soc == NULL)
		return -1;

	return register_filedescriptor(vfs_sock_create_conn(soc), 0);
}

int sock_accept(int soc, struct sockaddr *addr, socklen_t *addrlen)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		errno = ENOTSOCK;
		return -1;
	}

	if (!conn->soc->accept)
	{
		errno = ENOSYS;
		return -1;
	}

	struct socket_conn *accepted = conn->soc->accept(conn, addr, addrlen);
	return register_filedescriptor(vfs_sock_create_conn(accepted), 0);
}

int sock_bind(int soc, const struct sockaddr *addr, socklen_t adrlen)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		conn->soc = conn->create_socket(conn, (struct sockaddr *) addr, adrlen);
		if (conn->soc == NULL)
			return -1;
	}

	return conn->soc->bind(conn, addr, adrlen);
}

int sock_send(int soc, const void *buf, size_t len, int flags)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		errno = ENOTSOCK;
		return -1;
	}

	if (!conn->soc->send)
	{
		errno = ENOSYS;
		return -1;
	}

	return conn->soc->send(conn, buf, len, flags);
}

int sock_listen(int soc, int backlog)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		errno = ENOTSOCK;
		return -1;
	}

	if (!conn->soc->listen)
	{
		errno = ENOSYS;
		return -1;
	}

	return conn->soc->listen(conn, backlog);
}

int sock_recv(int soc, void *buf, size_t len, int flags)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		errno = ENOTSOCK;
		return -1;
	}

	if (!conn->soc->recv)
	{
		errno = ENOSYS;
		return -1;
	}

	return conn->soc->recv(conn, buf, len, flags);
}

int sock_connect(int soc, const struct sockaddr *addr, socklen_t addrlen)
{
	vfs_node_t *socknode = get_filedescriptor_node(soc);
	if (!socknode || !socknode->offset)
		return -1;

	struct socket_conn *conn = (struct socket_conn *) socknode->offset;
	if (!conn->soc)
	{
		if (sock_bind(soc, addr, addrlen) == -1)
			return -1;
	}

	if (!conn->soc->connect)
	{
		errno = ENOSYS;
		return -1;
	}

	return conn->soc->connect(conn, addr, addrlen);
}

int sock_close(vfs_node_t *socket)
{
	/* @todo */
	return 0;
}