#ifndef _LIBEVENT_HOOKS_H
#define _LIBEVENT_HOOKS_H

struct le_event;

int libevent_init();
int libevent_exit();
int libevent_hook_kb(void (*ptr)(struct le_event*));

#endif /* _LIBEVENT_HOOKS_H */
