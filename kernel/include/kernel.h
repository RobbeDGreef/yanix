#ifndef KERNEL_H
#define KERNEL_H

#include <proc/tasking.h>
#include <yanix/kfunctions.h>

#define KERN_EMERG   "<0>"
#define KERN_ALERT   "<1>"
#define KERN_CRIT    "<2>"
#define KERN_ERR     "<3>"
#define KERN_WARNING "<4>"
#define KERN_NOTICE  "<5>"
#define KERN_INFO    "<6>"
#define KERN_DEBUG   "<7>"

int  printk(const char *__restrict fmt, ...);
void clear_screenk();

#endif