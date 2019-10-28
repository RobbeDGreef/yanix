#ifndef ARP_H
#define ARP_H

#include <stdint.h>

#define ARP_HTYPE_ETHERNET 	1

#define ARP_PTYPE_IPV4		0x800
#define ARP_PTYPE_IPV6		0x806

#define ARP_REQUEST			1
#define ARP_REPLY			2

/*
 * @note: 	this is a ipv4 structure because
 * 			target_paddr, sender_paddr are 4 bytes
*/
typedef struct {
	uint16_t 	htype;			// hardware type (ethernet ...)
	uint16_t 	ptype;			// protocol type
	uint8_t		hlen;			// hardware address length
	uint8_t		plen;			// protocol address length
	uint16_t	operation;	
	uint8_t		source_mac[6];
	uint32_t 	source_paddr; 	// sender protocol address
	uint8_t 	target_mac[6];
	uint32_t	target_paddr;	// destination protocol address

} arp_frame_t;

#endif