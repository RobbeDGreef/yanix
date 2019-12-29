#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/**
 * @brief      Remaps the PIC
 *
 * @param[in]  master_offset  The master offset
 * @param[in]  slave_offset   The slave offset
 */
void pic_remap(uint8_t master_offset, uint8_t slave_offset);

void pic_send_eio(unsigned int irq);

#endif