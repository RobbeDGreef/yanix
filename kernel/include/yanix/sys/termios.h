#ifndef _YANIX_SYS_TERMIOS_H
#define _YANIX_SYS_TERMIOS_H

/* tcsetattr */
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#define TCOOFF 0
#define TCOON  1
#define TCIOFF 2
#define TCION  3

#define ISIG   00001
#define ICANON 00002

/* c_iflag bits */
#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IUCLC   0001000
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IMAXBEL 0020000
#define IUTF8   0040000

#define CSIZE  0000060
#define CS5    0000000
#define CS6    0000020
#define CS7    0000040
#define CS8    0000060
#define CSTOPB 0000100
#define CREAD  0000200
#define PARENB 0000400
#define PARODD 0001000
#define HUPCL  0002000
#define CLOCAL 0004000

#define ECHO   0000010
#define ECHOE  0000020
#define ECHOK  0000040
#define ECHONL 0000100
#define NOFLSH 0000200
#define TOSTOP 0000400

/* c_cc characters */
#define VINTR    0
#define VQUIT    1
#define VERASE   2
#define VKILL    3
#define VEOF     4
#define VTIME    5
#define VMIN     6
#define VSWTC    7
#define VSTART   8
#define VSTOP    9
#define VSUSP    10
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16

typedef unsigned char cc_t;
typedef unsigned int  speed_t;
typedef unsigned int  tcflag_t;

/* I just defined these like this as stubs,
 * i have no clue what they have to be */
#define TCGETS   0
#define TCSETS   1
#define TCXONC   2
#define TCFLSH   3
#define TCSETSW  4
#define TCSETSF  5
#define TCADRAIN 6

#define NCCS 32
struct termios
{
	tcflag_t c_iflag;    /* input mode flags */
	tcflag_t c_oflag;    /* output mode flags */
	tcflag_t c_cflag;    /* control mode flags */
	tcflag_t c_lflag;    /* local mode flags */
	cc_t     c_line;     /* line discipline */
	cc_t     c_cc[NCCS]; /* control characters */
	speed_t  c_ispeed;   /* input speed */
	speed_t  c_ospeed;   /* output speed */
};

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

#endif /* _YANIX_SYS_TERMIOS_H */