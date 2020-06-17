#ifndef _YANIX_SYS_IOCTL_H
#define _YANIX_SYS_IOCTL_H

#define FBIOGET_FSCREENINFO 1
#define FBIOGET_VSCREENINFO 2

#define TC_DISABLE 1
#define TC_ENABLE  2

int ioctl(int fd, int request, ...);

#endif /* _YANIX_SYS_IOCTL_H */