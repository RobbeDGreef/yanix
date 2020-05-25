#ifndef _STDIO_H
#define _STDIO_H

#include <fs/dirent.h>
#include <fs/vfs.h>

//#include <sys/cdefs.h>
#include <stdint.h>

#define BUFSIZ 0x400
#define EOF    -1
// standard library

int putchar(int);
int puts(const char *);
int readline(char *data, int dataend, char *buf, int size);

#endif