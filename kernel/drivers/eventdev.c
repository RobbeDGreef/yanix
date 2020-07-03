#include <kernel/atomic.h>
#include <fs/chardev.h>
#include <mm/heap.h>
#include <libk/string.h>
#include <libk/stdlib.h>
#include <fs/filedescriptor.h>
#include <debug.h>
unsigned long   g_eventdev_id = 0;
struct spinlock g_eventlock   = SPINLOCK_INIT;

int eventdev_create()
{
	spinlock_lock(&g_eventlock);

	char *event = "/dev/input/event";
	char  num[4];
	itoa(g_eventdev_id, num, 3);

	char *path = kmalloc(strlen(event) + strlen(num) + 1);
	strcpy(path, event);
	strcat(path, num);

	vfs_node_t *node = chardev_create(path, (void *) 1, (void *) 1, 0);
	g_eventdev_id++;

	spinlock_unlock(&g_eventlock);

	return register_filedescriptor(node, 0, O_RDWR);
}