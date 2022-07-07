#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <core.h>
#include <string.h>

static void *server_listener(void *arg)
{
	printf("Server listener\n");
}

static int spawn_listener(int sock)
{
	pthread_t listener;
	if (pthread_create(&listener, NULL, &server_listener, (void*) sock))
	{
		perror("Error creating thread, unable to recover and exiting");
		exit(1);
	}

	return 0;
}

int server_init(char *sockpath)
{
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		return -1;

	struct sockaddr_un serv;
	serv.sun_family = AF_UNIX;
	strcpy(serv.sun_path, sockpath);

	if (bind(sock, (struct sockaddr*) &serv, sizeof(struct sockaddr_un)))
		return -1;

	listen(sock, 5);

	spawn_listener(sock);
	
	return 0;
}
