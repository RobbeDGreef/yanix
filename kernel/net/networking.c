#include <net/networking.h>
#include <net/ethernet.h>
#include <net/arp.h>
#include <mm/heap.h>

networking_device_t *g_networking_devices = 0;

void add_networking_device(networking_device_t *netdev)
{
	if (g_networking_devices == 0) {
		g_networking_devices = netdev;
	} else {
		networking_device_t *tmp = g_networking_devices;
		while ((tmp = tmp->next) != 0) { /* loop over all devices */ }
		tmp->next = netdev;
	}
	return;
}


/**
 * @brief      Sends a raw packet.
 *
 * @param      netdev  The networking device
 * @param      packet  The packet
 * @param[in]  size    The size
 *
 * @return     Amount of bytes send or -1 on failure
 */
ssize_t send_raw_packet(networking_device_t *netdev, void *packet, size_t size)
{
	return netdev->send(packet, size);
}

#include <debug.h>

ssize_t send_ethernet_packet(char *dest_mac, uint16_t ether_type, void *payload, size_t size)
{
	void *packet = create_ethernet_packet(g_networking_devices, dest_mac, ether_type, payload, size);
	send_raw_packet(g_networking_devices, packet, size + ETHERNET_LAYER_SIZE);
	return  size + ETHERNET_LAYER_SIZE;
}

#define ETHER_LAYER 		(1 << 0)
#define IPV4_LAYER			(1 << 1)
#define IPV6_LAYER 			(1 << 2)
#define TCP_LAYER			(1 << 3)
#define ARP_LAYER 			(1 << 4)
#define UDP_LAYER			(1 << 5)

#define MIN_PAYLOAD_SIZE 	46

#include <libk/string.h>

#if 0

static uint32_t ether_frame_build(ethernet_frame_t *packet, uint16_t ether_type, char *receiving_mac, char *sending_mac)
{
	memcpy(packet->dest_mac, receiving_mac, 6);
	memcpy(packet->source_mac, sending_mac, 6);
	packet->type = ether_type;
	return sizeof(ethernet_frame_t) - 1;
}

ssize_t send_arp_request()

void *build_packet(networking_device_t *netdev, char *receiving_mac, uint32_t layers, uint32_t layer_values, void *payload, size_t payload_size)
{
	/* every packet starts at least with a ethernet layer */
	ethernet_frame_t *packet;
	uint32_t packet_offset = 0;

	/* first check on the highest layer */
	if (layers & TCP_LAYER) {
		// this is a tcp packet

	} else if (layers & UDP_LAYER) {
		// this is a udp packet

	} else if (layers & ARP_LAYER) {
		// this is a arp packet
		packet = kcalloc(ETHERNET_LAYER_SIZE + ARP_LAYER, 0); // arp packets do not cary a payload
		packet_offset = ether_frame_build(packet, ETHER_TYPE_ARP, receiving_mac, netdev->mac);
		
		arp_frame_t *arppacket = (arp_frame_t *) ((uint32_t) packet + packet_offset);

		arppacket->htype = ARP_HTYPE_ETHERNET; /* @TODO: this should be read out of layer_values */
		arppacket->ptype = ARP_PTYPE_IPV4; // (also todo, do ipv6 too0)
		arppacket->hlen  = 6; // ethenet size (also todo)
		arppacket->plen  = 4;

		// we and with 0x3 -> 0b11 that way we extract the arp operation which is 1 or 2
		arppacket->operation = layer_values & 0x3; 

		memcpy(arppacket->source_mac, netdev->mac, 6);
		memcpy(arppacket->dest_mac, receiving_mac, 6);

		arppacket->source_paddr = netdev->ip;
		
		memcpy(&arppacket->target_paddr, payload, 4);

		// @TODO: WHAT THE FUCKKK this is way more complicated then i expected, oke it is really fcking late and im tired asf from surfing all day but still
		// 		  so please just figure out how this crap works in the morning or smth cuz imma go to bed 
		
		packet_offset += sizeof(arp_frame_t);

	} else if (layers & IPV6_LAYER) {
		// this is only a ip packet

	} else if (layers & ETHER_LAYER) {
		// this is only a ethernet packet
		packet = kmalloc(ETHERNET_LAYER_SIZE + payload_size);
		 
		// @TODO: are you sure that just an ethernet packet is 0x0000 ????
		ether_frame_build(packet, 0x0000, receiving_mac, netdev->mac);
		memcpy(&packet->payload, payload, payload_size);
		packet_offset = (sizeof(ethernet_frame_t) - 1) + payload_size;
	} else {
		// The fuck???
		// you want me to send only the raw packet???
		// or is this a mistake
	}

	// calculate frame check sequence
	// packet_offset should at this point, point to the last byte of the packet
	uint32_t *crc = (uint32_t*) ((uint32_t) packet + packet_offset); // the one is for the sizeof(ethernet_frame_t) because of the extra payload start address
	*crc = ethernet_calculate_crc((void*) ((uint32_t) packet + sizeof(ethernet_frame_t) - 1), packet_offset - (sizeof(ethernet_frame_t) - 1));
	return packet;
}

#endif 