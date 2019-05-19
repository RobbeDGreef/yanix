#ifndef IO_H
#define IO_H

unsigned char port_byte_in (unsigned short port);
void port_byte_out (unsigned short port, unsigned char data);
unsigned short port_word_in (unsigned short port);
void port_word_out (unsigned short port, unsigned short data);
void outb(unsigned short port, unsigned char data);
/*
unsigned char inb(unsigned short port);
void outl(unsigned short port, unsigned short data);
unsigned short inl(unsigned short port);
*/
void insl(unsigned short port, unsigned int buffer, unsigned long count);

#endif