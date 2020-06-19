#include <net/socket.h>
#include <net/socket/unix.h>
#include <mm/heap.h>
#include <fs/vfs.h>
#include <fs/vfs_node.h>
#include <errno.h>
#include <libk/string.h>
#include <fs/pipe.h>
#include <debug.h>
#include <kernel/ds/queue.h>

struct socket *unix_socket_setup(int type, int protocol);
int            unix_socket_tryconnect(struct socket_conn *conn);

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
	soc->soc->server_lock    = 1;
	return 0;
}

int unix_socket_connect(struct socket_conn *conn, const struct sockaddr *addr,
                        socklen_t addrlen)
{
	if (!conn->soc->server_lock)
	{
		printk("Connection refused\n");
		errno = ECONNREFUSED;
		return -1;
	}

	conn->client = 1;
	return unix_socket_tryconnect(conn);
}

int unix_socket_send(struct socket_conn *conn, const void *buf, size_t size,
                     int flags)
{
	if (!conn || !conn->soc)
		return -1;

	return pipe_write_raw(conn->send_pipe, buf, size);
}

int unix_socket_recv(struct socket_conn *conn, void *buf, size_t size,
                     int flags)
{
	if (!conn || !conn->soc)
		return -1;

	return pipe_read_raw(conn->recv_pipe, buf, size);
}

int unix_socket_tryconnect(struct socket_conn *conn)
{
	struct accept_conn *accept_conn = kmalloc(sizeof(struct accept_conn));

	accept_conn->lock        = 1;
	accept_conn->client_pipe = pipe_create();
	accept_conn->server_pipe = pipe_create();

	/* Disable pipe read blocking */
	accept_conn->client_pipe->flags ^= O_NONBLOCK;
	accept_conn->server_pipe->flags ^= O_NONBLOCK;

	if (queue_push(conn->soc->accept_queue, accept_conn) == -1)
	{
		pipe_destroy(accept_conn->client_pipe);
		pipe_destroy(accept_conn->server_pipe);
		kfree(accept_conn);
		return -1;
	}

	while (accept_conn->lock)
		;

	conn->send_pipe = accept_conn->server_pipe;
	conn->recv_pipe = accept_conn->client_pipe;
	kfree(accept_conn);
	return 0;
}

struct socket_conn *unix_socket_accept(struct socket_conn *conn,
                                       struct sockaddr *   addr,
                                       socklen_t *         addrlen)
{
	struct queue *q = conn->soc->accept_queue;

	/* Im just gonna spinlock this, to keep it simple */
	while (!q->size)
		;

	struct accept_conn *ac       = queue_pop(q);
	struct socket_conn *accepted = kmalloc(sizeof(struct socket_conn));
	memcpy(accepted, conn, sizeof(struct socket_conn));

	accepted->send_pipe = ac->client_pipe;
	accepted->recv_pipe = ac->server_pipe;
	ac->lock            = 0;

	return accepted;
}

struct socket *unix_socket_setup(int type, int protocol)
{
	struct socket *soc = kmalloc(sizeof(struct socket));
	memset(soc, 0, sizeof(struct socket));

	soc->type     = type;
	soc->protocol = protocol;

	soc->accept_queue = queue_create(-1);

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
