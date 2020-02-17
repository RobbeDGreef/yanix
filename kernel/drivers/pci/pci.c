#include <drivers/pci.h>
#include <cpu/io.h>
#include <mm/heap.h>

#include <libk/string.h>

#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <debug.h>

// drivers
#include <drivers/networking/rtl8139.h>
#include <drivers/ata.h>

#define CONFIG_ADDRESS 	0xCF8
#define CONFIG_DATA 	0xCFC
/**
 * System to easily insert known devices with driver
 */

typedef int (*device_driver_fpointer) (pci_device_t *);

typedef struct known_device_s {
	unsigned int 			vendor_id;
	unsigned int 			device_id;
	
	device_driver_fpointer	driver;
	char 					*device_name;
} known_device_t;

typedef struct generic_device_s
{
	unsigned int 			class_code;
	unsigned int 			subclass;
	int 					prog_if;

	device_driver_fpointer	driver;
	char 					*device_name;
} generic_device_t;


/* Add entries these this lists */
const known_device_t known_devices[] = {
	{.vendor_id = 0x10EC, .device_id = 0x8139, .driver = &init_rtl8139, .device_name = "rtl8139 ethernet card"}
};

const generic_device_t generic_devices[] = {
	{.class_code = 0x01, .subclass = 0x01, .prog_if = -1, .driver = &init_ata, .device_name = "PATA hard drive"}
};

const size_t known_device_amount   = sizeof(known_devices)   / sizeof(known_device_t);
const size_t generic_device_amount = sizeof(generic_devices) / sizeof(generic_device_t);


/* Global variabels for the linked list of pci devices */
pci_device_t *g_pcilist;
unsigned int g_pci_device_count = 0;

static uint32_t make_pci_addr(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
	uint32_t lbus =  (uint32_t) bus;
	uint32_t lslot = (uint32_t) slot;
	uint32_t lfunc = (uint32_t) func;

	return  (uint32_t) ((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
}


uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
        uint32_t lbus =  (uint32_t) bus;
        uint32_t lslot = (uint32_t) slot;
        uint32_t lfunc = (uint32_t) func;

        uint16_t tmp;
        uint32_t addr = (uint32_t) ((lbus << 16) | (lslot << 11) | (lfunc << 8)| (offset & 0xFC) | ((uint32_t)0x80000000));

        port_line_out(0xCF8, addr);
        tmp = (uint16_t)((port_line_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
        return tmp;
}

#if 0
/**
 * @brief      Reads a word from the pci config memory
 *
 * @param[in]  bus     The bus
 * @param[in]  slot    The slot
 * @param[in]  func    The function
 * @param[in]  offset  The offset
 *
 * @return     The word read
 */
uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
	port_line_out(CONFIG_ADDRESS, make_pci_addr(bus, slot, func, offset));
	return (uint16_t)((port_line_in(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}
#endif

/**
 * @brief      Writes 32bits to the pci config memory
 *
 * @param[in]  bus       The bus
 * @param[in]  slot      The slot
 * @param[in]  function  The function
 * @param[in]  offset    The offset
 * @param[in]  value     The value
 */
void pci_write_dword(uint16_t bus, uint16_t slot, uint16_t function, uint16_t offset, uint32_t value) 
{
	port_line_out(CONFIG_ADDRESS, make_pci_addr(bus, slot, function, offset));
	port_line_out(CONFIG_DATA, value);
}

/**
 * @brief      Reads a word from a pcidevice 
 *
 * @param      pcidev  The pcidev
 * @param[in]  offset  The offset
 *
 * @return     The word read
 */
uint16_t pci_read(pci_device_t *pcidev, uint32_t offset)
{
	return pci_read_word(pcidev->bus, pcidev->device, pcidev->function, offset);
}

/**
 * @brief      Gets the pci vendor id
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 *
 * @return     The pci vendor id
 */
uint16_t pci_get_vendor_id(uint16_t bus, uint16_t device, uint16_t function)
{
	return (uint16_t) pci_read_word(bus, device, function, PCI_VENDOR_OFFSET); // 0
}

uint16_t pci_get_command(uint16_t bus, uint16_t device, uint16_t function)
{
	return (uint16_t) pci_read_word(bus, device, function, PCI_COMMAND_OFFSET); // 10
}

/**
 * @brief      Get the pci device id
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 *
 * @return     The pci device id
 */
uint16_t pci_get_device_id(uint16_t bus, uint16_t device, uint16_t function)
{
	return (uint16_t) pci_read_word(bus, device, function, PCI_DEVICE_ID_OFFSET); // 2
}

/**
 * @brief      Get the pci class id
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 *
 * @return     The pci class id
 */
uint16_t pci_get_class_id(uint16_t bus, uint16_t device, uint16_t function)
{
	return (((uint16_t) pci_read_word(bus, device, function, 0xA)) & ~0x00FF ) >> 8;
}


/**
 * @brief      Get the pci subclass id
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 *
 * @return     The pci subclass id
 */
uint16_t pci_get_subclass_id(uint16_t bus, uint16_t device, uint16_t function)
{
	return ((uint16_t) pci_read_word(bus, device, function, 0xA)) & ~0xFF00;
}

/**
 * @brief      Get the pci prog if
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 *
 * @return     The pci prog if
 */
uint16_t pci_get_prog_if(uint16_t bus, uint16_t device, uint16_t function)
{
	return (((uint16_t) pci_read_word(bus, device, function, 0x8)) & ~0x00FF ) >> 8;
}

/**
 * @brief      Gets a base address
 *
 * @param[in]  bus       The bus
 * @param[in]  device    The device
 * @param[in]  function  The function
 * @param[in]  address   The base address index
 *
 * @return     The value of the requested base address
 */
uint16_t pci_get_base_address(uint16_t bus, uint16_t device, uint16_t function, int address)
{
	uint32_t tmp = ((uint32_t) pci_read_word(bus, device, function, 0xf + address*4)) << 16;
	tmp |= (uint32_t) ((uint16_t) pci_read_word(bus, device, function, 0xf + address*4 + 2));
	return tmp;
}

uint16_t pci_get_header_type(uint16_t bus, uint16_t device, uint16_t function) 
{
	return (((uint16_t) pci_read_word(bus, device, function, 0xE)) & ~0xFF00 );
}

uint16_t pci_get_rev_id(uint16_t bus, uint16_t device, uint16_t function) 
{
	return ((uint16_t) pci_read_word(bus, device, function, 0x8)) & ~0xFF00;
}



void *pci_parse_header(uint16_t bus, uint16_t slot, uint16_t function, uint16_t header_type)
{
	if (header_type == 0x00) {
		pci_common_header_t *header = kcalloc(sizeof(pci_common_header_t), 0);
		for (size_t i = 0; i < 6; i++) {
			header->BAR[i] = pci_get_base_address(bus, slot, function, i);
		}
		// @TODO: a lot of other values should be set (they are 0 for now)
		header->interrupt_pin  = pci_read_word(bus, slot, function, PCI_INTERRUPT_PIN_OFFSET);
		header->interrupt_line = pci_read_word(bus, slot, function, PCI_INTERRUPT_LINE_OFFSET);
		return (void*) header;
	
	} else {
		// @TODO: implement the other types of headers too
		return 0;
	}
}

/**
 * @brief      Returns the bar value by index
 *
 * @param      pcidev  The pci device
 * @param[in]  index   The index
 *
 * @return     BAR value
 */
uint32_t pci_get_bar_by_index(pci_device_t *pcidev, uint32_t index)
{
	return (((pci_common_header_t*) pcidev->header)->BAR[index]);
}

/**
 * @brief      Gets a bar by type
 *
 * @param      pcidev  The pci device
 * @param[in]  type    The type
 *
 * @return     The bar value, else 0xFFFFFFFF
 */
uint32_t pci_get_bar_by_type(pci_device_t *pcidev, uint8_t type)
{
	uint32_t bar = 0;
	for (size_t i = 0; i < 6; i++) {
		bar = ((pci_common_header_t*) pcidev->header)->BAR[i];
		if ((bar & 0x1) == type) {
			return bar;
		}
	}
	// didn't find bar type
	return 0xFFFFFFFF;
}

/**
 * @brief      Adds a pci device.
 *
 * @param      dev   The device
 */
void add_pci_device(pci_device_t *dev) 
{
	if (g_pcilist == 0) {
		g_pcilist = dev;
	} else {
		volatile pci_device_t *tmp = g_pcilist;
		while (tmp->next != 0){tmp = tmp->next;}
		tmp->next = dev;
	}
	g_pci_device_count++;
}

#include <debug.h>

/**
 * @brief      Finds all the pci devices (bruteforce technique)
 */
static void _pci_find_brute()
{
	for (size_t bus = 0; bus < 256; bus++) {
		for (size_t slot = 0; slot < 32; slot++) {
			for(size_t function = 0; function < 8; function++) {
				uint16_t vendor_id = pci_get_vendor_id(bus, slot, function);
				if (vendor_id == 0xffff) {
					continue;
				}
				// found a device so now creating the structures
				pci_device_t *pcidev = (pci_device_t *) kcalloc(sizeof(pci_device_t), 0); // future proof, if i add entries to the struct and forget them they will automattically be 0
				
				// setting the values
				pcidev->bus = bus;
				pcidev->device = slot;
				pcidev->function = function;
				pcidev->vendor_id = vendor_id;
				pcidev->device_id = pci_get_device_id(bus, slot, function);
				pcidev->command = pci_get_command(bus, slot, function);
				pcidev->class_code = pci_get_class_id(bus, slot, function);
				pcidev->subclass = pci_get_subclass_id(bus, slot, function);
				pcidev->prog_if = pci_get_prog_if(bus, slot, function);
				pcidev->header_type = pci_get_header_type(bus, slot, function);
				pcidev->header = pci_parse_header(bus, slot, function, pcidev->header_type);
				pcidev->rev_id = pci_get_rev_id(bus, slot, function);
				pcidev->name = "Unknown pci device";
				add_pci_device(pcidev);

				printk(KERN_DEBUG "PCI device classcode: %i subclass %i prog if %i\n", pcidev->class_code, pcidev->subclass, pcidev->prog_if);

				/* and now check if it is present in the drivers list */
				for (size_t i = 0; i < known_device_amount; i++) {
					if (known_devices[i].vendor_id == pcidev->vendor_id && known_devices[i].device_id == pcidev->device_id) {
						// found a device
						pcidev->driver = known_devices[i].driver;
						pcidev->name   = known_devices[i].device_name;
					}
				}

				for (size_t i = 0; i < generic_device_amount; i++)
				{
					if (generic_devices[i].class_code == pcidev->class_code && generic_devices[i].subclass == pcidev->subclass)
					{
						if (generic_devices[i].prog_if == -1 || generic_devices[i].prog_if == pcidev->prog_if)
						{
							pcidev->name = generic_devices[i].device_name;
							pcidev->driver = generic_devices[i].driver;
						}
					}
				}
			}
		}
	}
}

/**
 * @brief      Initialises all the pci devices with a driver
 */
int init_pci_devices()
{
	pci_device_t *tmp = g_pcilist;
	
	if (tmp == 0)
		return 0;

	int ret = 0;
	do 
	{
		if (tmp->driver != 0)
		{
			ret = tmp->driver(tmp);
			printk("[ PCI ] Initialisation of %s", (char*)tmp->name); printk(ret?" failed\n":" succeeded\n");
		}
	} while ((tmp = tmp->next) != 0);
	return 0;
}

/**
 * @brief      Finds a pci device by classcode, subclass and prog_if
 *
 * @param[in]  classcode  The classcode
 * @param[in]  subclass   The subclass
 * @param[in]  prog_if    The prog if
 *
 * @return     Pointer to pci device struct, otherwise 0
 */
pci_device_t *pci_find_by_class(int classcode, int subclass, int prog_if)
{
	/* No pci devices ?? */
	if (g_pcilist == 0)
		return 0;

	pci_device_t *tmp = g_pcilist;
	do 
	{
		if (tmp->class_code == classcode && tmp->subclass == subclass && tmp->prog_if == prog_if)
			return tmp; 
		
		tmp = tmp->next;
	} while (tmp != 0);

	/* pci device not found returning 0 */
	return 0; 
}

/**
 * @brief      Finds a pci device by vendor and device ids
 *
 * @param[in]  vendor  The vendor
 * @param[in]  device  The device
 *
 * @return     { description_of_the_return_value }
 */
pci_device_t *pci_find_by_vendor(int vendor, int device)
{
	/* No pci devices ?? */
	if (g_pcilist == 0)
		return 0;

	pci_device_t *tmp = g_pcilist;
	do 
	{
		if (tmp->vendor_id == vendor && tmp->device_id == device)
			return tmp;
		
		tmp = tmp->next;
	} while (tmp != 0);
	
	/* pci device not found returning 0 */
	return 0; 
}

/**
 * @brief      Initialises pci devices (finds them)
 */
int init_pci()
{
	_pci_find_brute();
	
	/* Nothing can really go wrong here so we don't need to do any error checking */
	return 0;

}
