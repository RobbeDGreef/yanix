#ifndef _LIBK_ASSERT_H
#define _LIBK_ASSERT_H

#include <kernel.h>

#define assert(expr) if (!(expr)) { printk("Assertion botched at line %i in file %s\n", __LINE__, __FILE__); for(;;);}

#endif /* _LIBK_ASSERT_H */