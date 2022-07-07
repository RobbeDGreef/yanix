#include <core.h>
#include <renderer.h>
#include <server.h>
#include <window.h>

void mainloop_start();

int main()
{
	printf("Started yanix winserver\n");

	server_init(DEFAULT_SERV_SOCKPATH);

	if (renderer_init())
		printf("A well shit ay\n");

	yws_create_simple("Hello world window", 500,300);
	yws_create("Other window", 200, 200, 500, 300);

	printf("Spawned window");

	mainloop_start();
}
