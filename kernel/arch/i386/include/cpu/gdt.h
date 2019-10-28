#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/**
 * @brief      Initializes the descriptor tables.
 */
void init_descriptor_tables();

/**
 * @brief      Sets the tss kernel stack
 *
 * @param[in]  stack  The kernel stack
 */
void tss_set_kernel_stack(uint32_t stack);


#endif