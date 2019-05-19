unsigned char port_byte_in(unsigned short port){
        unsigned char result;
        __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
        return result;
}

void port_byte_out(unsigned short port, unsigned char data){
        __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

unsigned short port_word_in(unsigned short port){
        unsigned short result;
        __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
        return result;
}

void port_word_out(unsigned short port, unsigned short data){
        __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

/*
void outl(unsigned short port, unsigned short data){
    asm ( "outl %0, %1" : : "a"(data), "Nd"(port) );
}

unsigned short inl(unsigned short port){
    unsigned short ret;
    asm ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}
*/
void insl(unsigned short port, unsigned int buffer, unsigned long count){
    asm("cld; rep; insl":: "D"(buffer), "d"(port), "c"(count));
}
