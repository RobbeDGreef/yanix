/**
 * @defgroup   SERIAL serial
 *
 * @brief      This file implements a very serial driver.
 *
 * @author     Robbe De Greef
 * @date       2019
 */

#include <cpu/io.h>
#include <stddef.h>

#define SERIAL_PORT 	0x3F8

#define SERIAL_CMD_DISABL_INT 			0x00
#define SERIAL_CMD_ENABLE_DLAB 			0x80
#define SERIAL_CMD_ENABLE_FIFO 			0xC7
#define SERIAL_CMD_ENABLE_IRQ_RTS_DTS	(1 << 3) | (1 << 1) | (1 << 0)
#define MAX_SERIAL_BAUD 				115200

void init_serial()
{
	port_byte_out(SERIAL_PORT + 1, 0x00); 	/* Disable all interrupts */
	port_byte_out(SERIAL_PORT + 3, SERIAL_CMD_DISABL_INT);
	port_byte_out(SERIAL_PORT + 0, 3);		/* Set divisor to 3 (38400 baud) */
	port_byte_out(SERIAL_PORT + 1, 0x00); 	/* hi byte */
	port_byte_out(SERIAL_PORT + 3, 0x03); 	/* no parity and one stop bit*/
	port_byte_out(SERIAL_PORT + 2, SERIAL_CMD_ENABLE_FIFO);
	port_byte_out(SERIAL_PORT + 4, SERIAL_CMD_ENABLE_IRQ_RTS_DTS);
}

int check_data_received()
{
	return port_byte_in(SERIAL_PORT + 5) & 1;
}

/**
 * @brief      Reads from a serial connection.
 *
 * @return     The read data
 */
char serial_read()
{
	while (check_data_received() == 0);
	return port_byte_in(SERIAL_PORT);
}

int check_can_transmit()
{
	return port_byte_in(SERIAL_PORT + 5) & 0x20;
}

/**
 * @brief      Writes to serial connection in bytes.
 *
 * @param[in]  data  The data
 */
void serial_put(char data)
{
	while (check_can_transmit() == 0);

	port_byte_out(SERIAL_PORT, data);
}

/**
 * @brief      Writes to a serial connection in c style strings
 *
 * @param      data  The data
 */
void serial_write(char *data)
{
	char c;
	size_t i = 0;
	while ((c = data[i]) != 0)
	{
		serial_put(c);
		i++;
	}
}