#ifndef KCONFIG_H
#define KCONFIG_H

// the time the os waits after bootsequence is completed
#define TIMEAFTERBOOT 500

extern void (*KeyboardHook)(char character);

// keyboard list

#include <config/belgian.h> // this is a hack and it should be loaded in dynamicly but that's a todo

#endif