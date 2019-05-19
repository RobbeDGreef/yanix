#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

/*
extern int16_t   mouse_xrel;
extern int16_t   mouse_yrel;
extern int32_t  mouse_xloc;
extern int32_t  mouse_yloc;
extern uint8_t mouse_lbut;
extern uint8_t mouse_lheld;
extern uint8_t mouse_rbut;
extern uint8_t mouse_mbut;
*/

typedef void (*mousehook_fpointer)(int16_t xrel, int16_t yrel, uint16_t xloc, uint16_t yloc, int lbut, int rbut, int mbut);

void init_mouse();
void mouse_poll();
uint8_t register_mouse_hook(mousehook_fpointer hook);
uint8_t remove_mouse_hook(mousehook_fpointer hook);
void mouse_loop();


#endif