#include <net/ethernet.h>
#include <drivers/pci/pci.h>
#include <drivers/networking/rtl8139.h>
#include <net/networking.h>
#include <mm/heap.h>
#include <libk/string/string.h>

#define PCI_NETWORK_CONTROLLER 			0x02
#define PCI_NETWORK_EHTERNET_SUBCLASS 	0x00

#include <debug.h>

/*
 * The ethernet preable are 7 bytes of 0b10101010 with a last byte of 0b10101011
*/
const uint8_t ethernet_preamble[8] = { 0xAA, 0xAA, 0xAA, 0xAA,
									   0xAA, 0xAA, 0xAA, 0xAB };

/**
 * @brief      Calculates the frame check sequence
 *
 * @param      payload  The payload
 * @param[in]  size     The size
 *
 * @return     Frame check sequence
 */
uint32_t ethernet_calculate_crc(void *payload, size_t size)
{
	(void) (payload); (void) (size);
	// @TODO: crc calculating
	return 0xFFFFFFFF;
}

/**
 * @brief      Creates an ethernet packet.
 *
 * @param[in]  netdev    The networking device
 * @param      dest_mac  The destination mac address
 * @param[in]  type      The type of packet (arp, ip ...)
 * @param      payload   The payload
 * @param[in]  size      The size
 * @param      buffer  The buffer
 *
 * @return     Pointer to end of ethernet frame
 */
void *create_ethernet_packet(networking_device_t *netdev, char *dest_mac, uint16_t type, void *payload, size_t size)
{
	// alocate the buffer for the frame
	printk("ethernet layer size: %u size of eth frame: %u\n", ETHERNET_LAYER_SIZE, sizeof(ethernet_frame_t));
	ethernet_frame_t *packet = kmalloc(ETHERNET_LAYER_SIZE + size);

	// copy some values into the packet, set the type
	memcpy(packet->dest_mac, dest_mac, 6);
	memcpy(packet->source_mac, netdev->mac, 6);
	packet->type = type;
	
	// copy the payload into the packet
	memcpy((void*) (((uint32_t) packet) + sizeof(ethernet_frame_t)-1), payload, size);

	// calculate frame check sequence
	uint32_t *crc = (uint32_t*) ((uint32_t) packet + sizeof(ethernet_frame_t) - 1 + size);
	
	// set frame check sequence
	*crc = ethernet_calculate_crc(payload, size);
	return packet;
}

