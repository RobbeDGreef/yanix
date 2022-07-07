#ifndef _LIBEVENT_BINDINGS_H
#define _LIBEVENT_BINDINGS_H

struct le_event;

int libevent_init();
int libevent_quit();
int libevent_hook_kb(void (*)(struct le_event*));

#endif /* _LIBEVENT_BINDINGS_H */
