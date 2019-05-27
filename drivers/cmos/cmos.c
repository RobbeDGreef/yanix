#include <drivers/io/io.h>

#define CMOS_SELECT_PORT	0x70
#define CMOS_IO_PORT 		0x71

/**
 * @brief      Function read from cmos address
 *
 * @param[in]  addr  The address
 *
 * @return     returns the read byte
 */
inline int cmos_read(int addr)
{
	port_byte_out(0x80|addr, CMOS_SELECT_PORT);
	return port_byte_in(CMOS_IO_PORT);
}

/**
 * @brief      Changes BCD values into binary ones
 *
 * @param[in]  val   The BCD value
 *
 * @return     The binary value
 */
int bcd_to_bin(int val)
{
	return (val&15) + ((val>>4)*10);
}