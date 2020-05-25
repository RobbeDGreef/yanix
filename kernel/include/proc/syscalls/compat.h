#ifndef _SYSCALL_COMPAT_H
#define _SYSCALL_COMPAT_H

extern void *syscalls_yanix[];
extern void *syscalls_linux[];
extern int syscall_linux_amount;
extern int syscall_yanix_amount;

#define COMPAT_YANIX 1
#define COMPAT_LINUX 2

#endif /* _SYSCALL_COMPAT_H */
