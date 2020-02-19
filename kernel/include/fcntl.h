#ifndef FCNTL_H
#define FCNTL_H

//@todo: incomplete 

#define O_ACCMODE	0x3
#define O_RDONLY	0x0
#define O_WRONLY	0x1
#define O_RDWR		0x2
#define O_CREAT		0x100
#define O_EXCL		0x200
#define O_NOCTTY	0x400
#define O_TRUNC		0x1000
#define O_APPEND	0x2000
#define O_NONBLOCK	0x4000
#define O_DSYNC		0x10000
#define FASYNC		0x20000
#define O_DIRECTORY 0x200000

#endif