#include <cpu/cpu.h>
#include <libk/function.h>
#include <drivers/video/video.h>
#include <cpu/io.h>
#include <libk/bit.h>
#include <libk/string.h>
#include <drivers/ps2/mouse.h>
#include <stdint.h>



int16_t   mouse_xrel;
int16_t   mouse_yrel;
int32_t  mouse_xloc;
int32_t  mouse_yloc;
int mouse_lbut;
int mouse_rbut;
int mouse_mbut;
int mouse_lheld;


uint8_t MOUSELOOPING       = 0;
uint32_t MOUSELOOPINTERVAL = 0;

#define HOOKS       8
mousehook_fpointer  mousehooks[HOOKS];   // max of 8 hooks

uint8_t register_mouse_hook(mousehook_fpointer hook){
    for (int i = 0; i<HOOKS; i++){
        if (mousehooks[i] == 0){
            mousehooks[i] = hook;
            return 0;
        }
    }
    return 0; // all mouse pointer are already in use
}

uint8_t remove_mouse_hook(mousehook_fpointer hook){
    for (int i = 0; i<HOOKS; i++){
        if (mousehooks[i] == hook){
            mousehooks[i] = 0;
            return 0;
        }
    }
    return 1; // not found
}

static inline void mouse_wait(uint8_t a_type){
    uint32_t _time_out = 100000;
    if(a_type==0) {
        while(_time_out--){
            if((port_byte_in(0x64) & 1) == 1){
                return;
            }
        }
        return;
    }
    else {
        while(_time_out--){
            if((port_byte_in(0x64) & 2) == 0){
                return;
            }
        }
    return;
    }
}

static inline void mouse_write(uint8_t a_write){

    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    port_byte_out(0x64, 0xD4);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    port_byte_out(0x60, a_write);
}

uint8_t mouse_read()
{
    //Get's response from mouse
    mouse_wait(0);
    return port_byte_in(0x60);
}

void mouse_poll(){
    uint8_t out = port_byte_in(0x64);
    if (out == 61){                     // BUG: or yeah not the right way to go about thing is think (should only check 1 bit or something)
                
        uint8_t byte1 = port_byte_in(0x60);  // buttons
        if (mouse_lbut && getbit(byte1, 0)){
            mouse_lheld = 1;
        } else {
            mouse_lheld = 0;
        }
        
        mouse_lbut = getbit(byte1, 0);
        mouse_rbut = getbit(byte1, 1);
        mouse_mbut = getbit(byte1, 2);
        
        mouse_xrel = (int8_t) port_byte_in(0x60);  // x movement
        mouse_yrel = (int8_t) port_byte_in(0x60);  // y movement
        
        // todo: check that we do not go out of bounds
        
        if ((mouse_xloc+mouse_xrel) > 0 && (mouse_xloc + mouse_xrel) < video_get_screen_width()){
            mouse_xloc += mouse_xrel;
        }
        if ((mouse_yloc-mouse_yrel) > 0 && (mouse_yloc - mouse_yrel) < video_get_screen_height()){
            mouse_yloc -= mouse_yrel;
        }
        
    }
}

void init_mouse()
{
    MOUSELOOPING = 1;
    MOUSELOOPINTERVAL = 10;
    memset((uint32_t*) mousehooks, 0, sizeof(mousehook_fpointer)*HOOKS);
    uint8_t _status;
    //Enable the auxiliary mouse device
    mouse_wait(1);
    port_byte_out(0x64, 0xA8);
 
    //Enable the interrupts
    mouse_wait(1);
    port_byte_out(0x64, 0x20);
    mouse_wait(0);
    _status = (port_byte_in(0x60) | 2);
    mouse_wait(1);
    port_byte_out(0x64, 0x60);
    mouse_wait(1);
    port_byte_out(0x60, _status);
 
    //Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();

    //Enable the mouse
    mouse_write(0xF4);
    mouse_read();
}


void mouse_loop(){
    while (MOUSELOOPING){
        mouse_poll();
        //print("X: ");print_hex(mouse_xloc);print(" Y: "); print_hex(mouse_yloc);print("\n");
        //drawrect(mouse_xloc, mouse_yloc, 10,10,0xff00ff);
        for (int i = 0; i<HOOKS; i++){
            if (mousehooks[i] != 0){
                mousehooks[i](mouse_xrel,mouse_yrel, mouse_xloc, mouse_yloc, mouse_lbut, mouse_rbut, mouse_mbut);
            }
        }
        
        //sleep(MOUSELOOPINTERVAL);
    }
}