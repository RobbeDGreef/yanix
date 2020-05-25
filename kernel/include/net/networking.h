#ifndef NETWORKING_H
#define NETWORKING_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct networking_device_s;
typedef struct networking_device_s networking_device_t;

typedef size_t (*net_send_t)(const void *buf, size_t size);

struct networking_device_s
{
	void *               device; // device (for exaple the e1000_device_t)
	net_send_t           send;
	char                 mac[6];
	int                  use_ipv6;
	uint32_t             ip;
	uint8_t              ipv6[6];
	uint32_t             dns_ip;
	uint32_t             subnet_mask;
	networking_device_t *next;
};

void    add_networking_device(networking_device_t *netdev);
ssize_t send_raw_packet(networking_device_t *netdev, void *packet, size_t size);
ssize_t send_ethernet_packet(char *dest_mac, uint16_t ether_type, void *payload,
                             size_t size);

void *build_packet(networking_device_t *netdev, char *receiving_mac,
                   uint32_t layers, uint32_t layer_values, void *payload,
                   size_t payload_size);

#endif