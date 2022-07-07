#ifndef _WS_RENDERER_H
#define _WS_RENDERER_H

struct ywindow;
int renderer_pushwin(struct ywindow*);
int renderer_init();
void renderer_render();

#endif /* _WS_RENDERER_H */
