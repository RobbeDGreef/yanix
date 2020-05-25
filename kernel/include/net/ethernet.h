#ifndef ETHERNET_H
#define ETHERNET_H

#include <net/networking.h>

/* This is ethernet 1 frame, now we all use ethernet 2 frames so this is
 * obselete?? */
#if 0
typedef struct {
	uint8_t		preamble[8];
	uint8_t		dest_mac[6];
	uint8_t 	source_mac[6];
	uint16_t	type;
	// ... data
	// frame check sequence CRC (uint32_t)
} ethernet_frame_t;
#endif

typedef struct
{
	uint8_t  dest_mac[6];
	uint8_t  source_mac[6];
	uint16_t type;
	uint8_t  payload; // this is just an easy start address for memcpys
					  // frame check sequence CRC (uint32_t)
} ethernet_frame_t;

#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_ARP  0x0806
#define ETHER_TYPE_IPV6 0x86DD

#define ETHERNET_LAYER_SIZE \
	sizeof(ethernet_frame_t) + 4 // +4 = frame check sequence

/**
 * @brief      Creates an ethernet packet.
 *
 * @param      buffer    The buffer
 * @param[in]  netdev    The networking device
 * @param      dest_mac  The destination mac address
 *
 * @return     Pointer to end of ethernet frame
 */
void *create_ethernet_packet(networking_device_t *netdev, char *dest_mac,
                             uint16_t type, void *payload, size_t size);

/**
 * @brief      Calculates the frame check sequence
 *
 * @param      payload  The payload
 * @param[in]  size     The size
 *
 * @return     Frame check sequence
 */
uint32_t ethernet_calculate_crc(void *payload, size_t size);

#endif