#ifndef SERIAL_H
#define SERIAL_H

/**
 * @brief      Reads from a serial connection.
 *
 * @return     The read data
 */
char serial_read();

/**
 * @brief      Writes to serial connection in bytes.
 *
 * @param[in]  data  The data
 */
void serial_put(char data);

/**
 * @brief      Writes to a serial connection in c style strings
 *
 * @param      data  The data
 */
void serial_write(char *data);

void init_serial();

#endif