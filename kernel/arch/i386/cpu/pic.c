/**
 * @defgroup   DRIVERS pic
 *
 * @brief      This file implements the PIC driver.
 *
 * @author     Robbe De Greef
 * @date       2019
 */
#include <cpu/io.h>
#include <cpu/idt.h>
#include <stdint.h>

/* PIC io ports */
#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

/* PIC commands */
#define PIC_CMD_EOI            (1 << 5) /* End of interrupt command */
#define PIC_CMD_ICW1_ICW4      (1 << 0) /* ICW4 (not) needed */
#define PIC_CMD_ICW1_SINGLE    (1 << 1) /* Single (cascade) mode */
#define PIC_CMD_ICW1_INTERVAL4 (1 << 2) /* Call address interval 4 */
#define PIC_CMD_ICW1_LEVEL     (1 << 3) /* Level triggered (edge) mode */
#define PIC_CMD_ICW1_INIT      (1 << 4) /* Initialization */

#define PIC_CMD_ICW4_8086       (1 << 0) /* 8086/88 mode */
#define PIC_CMD_ICW4_AUTO       (1 << 1) /* Auto normal end of interrupt */
#define PIC_CMD_ICW4_BUF_SLAVE  (1 << 3) /* Buffered slave mode */
#define PIC_CMD_ICW4_SFNM       (1 << 4) /* Special fully nested */
#define PIC_CMD_ICW4_BUF_MASTER (1 << 2) | (1 << 3) /* Buffered master mode */

#define PIC_CMD_SLAVE_AT_IRQ2 (1 << 2)
#define PIC_CMD_CASCADE_ID    (1 << 1)

/**
 * @brief      Remaps the PIC
 *
 * @param[in]  master_offset  The master offset
 * @param[in]  slave_offset   The slave offset
 */
void pic_remap(uint8_t master_offset, uint8_t slave_offset)
{
	/* Save the PIC masks */
	// uint8_t master_mask = port_byte_in(PIC_MASTER_DATA);
	// uint8_t slave_mask  = port_byte_in(PIC_SLAVE_DATA);

	/* Start init sequence */
	port_byte_out(PIC_MASTER_CMD, PIC_CMD_ICW1_INIT | PIC_CMD_ICW1_ICW4);
	port_byte_out(PIC_SLAVE_CMD, PIC_CMD_ICW1_INIT | PIC_CMD_ICW1_ICW4);

	/* Set PIC offsets */
	port_byte_out(PIC_MASTER_DATA, master_offset);
	port_byte_out(PIC_SLAVE_DATA, slave_offset);

	/* Tell PIC there is a slave at IRQ2 */
	port_byte_out(PIC_MASTER_DATA, PIC_CMD_SLAVE_AT_IRQ2);

	/* Tell slave PIC its cascade identity */
	port_byte_out(PIC_SLAVE_DATA, PIC_CMD_CASCADE_ID);

	/* Set PIC in 8086 mode */
	port_byte_out(PIC_MASTER_DATA, PIC_CMD_ICW4_8086);
	port_byte_out(PIC_SLAVE_DATA, PIC_CMD_ICW4_8086);

	/* Restore masks */
	port_byte_out(PIC_MASTER_DATA, 0);
	port_byte_out(PIC_SLAVE_DATA, 0);
}

/**
 * @brief      Send an end of interrupt command to the pic
 *
 * @param[in]  irq   The interrupt request number
 */
void pic_send_eio(unsigned int irq)
{
	if (irq >= 8)
		port_byte_out(PIC_SLAVE_CMD, PIC_CMD_EOI);

	port_byte_out(PIC_MASTER_CMD, PIC_CMD_EOI);
}

int init_pic()
{
	return 0;
}
