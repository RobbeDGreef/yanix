#ifndef _KERNEL_FB_H
#define _KERNEL_FB_H

#include <yanix/yanix/fb.h>
#include <fs/vfs_node.h>
#include <sys/types.h>

int     fb_cmd(int request, char *args);
ssize_t fb_write(vfs_node_t *node, unsigned int off, const void *buf,
                 size_t size);

#endif /* _KERNEL_FB_H */