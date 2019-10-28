#ifndef _CMOS_H
#define _CMOS_H

/**
 * @brief      Function read from cmos address
 *
 * @param[in]  addr  The address
 *
 * @return     returns the read byte
 */
int cmos_read(int addr);

/**
 * @brief      Changes BCD values into binary ones
 *
 * @param[in]  val   The BCD value
 *
 * @return     The binary value
 */
int bcd_to_bin(int val);

#endif