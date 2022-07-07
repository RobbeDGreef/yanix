#include <hooks.h>
#include <event.h>

#include <stdio.h>
#include <stdlib.h>

void testhk(struct le_event *ev)
{
	printf("got pkt %i\n", ev->kb.scancode);
}

int main()
{
	libevent_init();
	printf("I want a hook\n");
	libevent_hook_kb(testhk);
	for(;;);
	libevent_exit();
}
