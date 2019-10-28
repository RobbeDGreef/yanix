#include <drivers/pci.h>
#include <net/networking.h>
#include <drivers/networking/rtl8139.h>
#include <cpu/io.h>

#include <cpu/isr.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <libk/string.h>

#include <debug.h>

#include <stdint.h>
#include <stddef.h>

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define CARP 						0x38
#define RECEIVE_READ_POINTER_MASK  ~0x3
#define RECEIVER_BUF_SIZE 			0x2000

#define REG_MAC0_5	0x00
#define REG_MAR0_7	0x08
#define REG_RBSTART	0x30
#define REG_CMD		0x37
#define REG_IMR		0x3C
#define REG_ISR		0x3E

#define ROK_BIT (1 << 0)
#define TOK_BIT (1 << 2)

#define AB_AM_APM_AAP_BITS	0xF
#define WRAP_BITS 			(1 << 7)
#define RE_TE_BITS 			(1 << 2) | (1 << 3)

char TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
char TSD_array[4]  = {0x10, 0x14, 0x18, 0x1C};

extern page_directory_t *g_current_directory;

rtl8139_dev_t *g_rtl_dev;

void read_mac_address(networking_device_t *networkdev, rtl8139_dev_t *rtl_dev) 
{
	for (size_t i = 0; i < 6; i++) {
		networkdev->mac[i] = port_byte_in(rtl_dev->io_base + i);
	}

}

/**
 * @brief      Sends a packet.
 *
 * @param      data  The data
 * @param[in]  size  The size
 */
size_t send_packet(const void *data, size_t size)
{
	// copy the data in a block of memory
	uint32_t phys = 0; 
	void *transfer = kmalloc_base(size, 1, &phys);
	memcpy(transfer, data, size);

	// you must use a different TXAD register for each packet (so we just loop over the list constantly)
	port_line_out(g_rtl_dev->io_base + TSAD_array[g_rtl_dev->tx_current], phys);
	port_line_out(g_rtl_dev->io_base + TSD_array[g_rtl_dev->tx_current++], size);	

	// just loop around the array
	if (g_rtl_dev->tx_current > 3) {
		g_rtl_dev->tx_current = 0;
	}
	return size;
}

void receive_packet()
{
	uint16_t *transmission = (uint16_t*)((uint32_t) g_rtl_dev->rx_buffer + g_rtl_dev->current_packet_offset);
	uint16_t packet_size   = *(transmission + 1);
	uint16_t *packet_data  = transmission + 2;

	void *packet = kmalloc(sizeof(packet_size));
	memcpy(packet, packet_data, packet_size);
	printk_hd(packet, 0x50);

	g_rtl_dev->current_packet_offset = (g_rtl_dev->current_packet_offset+ packet_size + 4 + 3) & RECEIVE_READ_POINTER_MASK;

	if (g_rtl_dev->current_packet_offset > RECEIVER_BUF_SIZE) {
		g_rtl_dev->current_packet_offset -= RECEIVER_BUF_SIZE;
	}

	port_word_out(g_rtl_dev->io_base + CARP, g_rtl_dev->current_packet_offset - 0x10);

}

static void rtl8139_handler(registers_t *regs)
{
	(void) (regs);
	printk("got a rtl interrupt: "); 
	uint16_t status = port_word_in(g_rtl_dev->io_base + 0x3e);
	if (status & TOK_BIT) {
		printk("Packet send\n");
	} 
	if (status & ROK_BIT) {
		printk("Packet received\n");
		receive_packet();
	}
	// we need to reset the isr handler by writing to it
	port_word_out(g_rtl_dev->io_base + REG_ISR, 0x5);
}

/**
 * @brief      Initialises the rtl8139 ethernet card
 *
 * @return     0 on success, -1 on failure
 */
int init_rtl8139(pci_device_t *pci_dev)
{
	// allocate the structures
	networking_device_t *networkdev = kcalloc(sizeof(networking_device_t), 0);
	rtl8139_dev_t *rtl_dev = kmalloc(sizeof(rtl8139_dev_t));

	if ((networkdev == 0) | (rtl_dev == 0)) {
		return -1;
	}

	// set some values
	networkdev->device = rtl_dev;
	networkdev->send = &send_packet;
	g_rtl_dev = rtl_dev;

	networkdev->device = rtl_dev;
	rtl_dev->pci_dev = pci_dev;

	// getting the bar0 type and calculating io and mem addresses
 	uint32_t bar0 = pci_get_bar_by_index(rtl_dev->pci_dev, 0);
	rtl_dev->bar_type = bar0 & 0x1;  // first bit of bar0 holds type
	                                 // 
	// get the io and mem addresses by extracting the highest 28/30 bits
	rtl_dev->io_base  = bar0 & ~0x3;
	rtl_dev->mem_base = bar0 & ~0xf;

	// set the current TXAD array index to 0
	rtl_dev->tx_current = 0;

	// First step is enableing pci bus mastering so that we can preform DMA
	if (!(rtl_dev->pci_dev->command & (1 << 2))) {
		// if the bit isn't already set we need to set it
		rtl_dev->pci_dev->command |= (1 << 2);
		pci_write_dword(rtl_dev->pci_dev->bus, rtl_dev->pci_dev->device, rtl_dev->pci_dev->function, PCI_COMMAND_OFFSET, rtl_dev->pci_dev->command);
	}

	// activate the device
	port_byte_out(rtl_dev->io_base + 0x52, 0x0); // 0x52 is CONFIG_1 register

	// software reset to clear receive and transmit buffers
	port_byte_out(rtl_dev->io_base + REG_CMD, 0x10); 
	while ((port_byte_in(rtl_dev->io_base + 0x37) & 0x10) != 0) {
		// wait until active
	}

	//@FIXME: this could be a nasty bug to track, the pointer to the buffer we allcoated is a physical address
	// 		  in other words its possible that our mem manager sticked a couple of pages together to acchieve this
	//		  and we wouldn't have a continues buffer

	// now allocate the receiving buffer
	uint32_t phys;
	rtl_dev->rx_buffer = kmalloc_base(1024*8 + 16 + 1500, 1, &phys);
	memset(rtl_dev->rx_buffer, 0, 1024*8 + 16 + 1500);
	port_line_out(rtl_dev->io_base + REG_RBSTART, phys); // we need to get the physical address
		
	// Sets transmit OK and receive OK bits to True
	port_word_out(rtl_dev->io_base + REG_IMR, TOK_BIT | ROK_BIT); // 0b0101 

	// now configure the receiving buffer
	// setting WRAP, AB, AM, APM, AAP bits
	port_line_out(rtl_dev->io_base + 0x44, WRAP_BITS | AB_AM_APM_AAP_BITS);

	// setting TE and RE bits
	port_byte_out(rtl_dev->io_base + REG_CMD, 0x0C);

	// now register and enable interrupt handlers
	arch_register_interrupt_handler(((pci_common_header_t*)rtl_dev->pci_dev->header)->interrupt_line+32, &rtl8139_handler);
		
	// read the mac address
	read_mac_address(networkdev, rtl_dev);

	// add the working networking device to the networking devices list
	add_networking_device(networkdev);

	// completely initialsed without any problems
	return 0;
}