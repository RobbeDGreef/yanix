
#include <stdint.h>

typedef struct
{
	char    name[8];
	char    ext[3];
	uint8_t attrib;
	uint8_t userattrib;

	char     undelete;
	uint16_t createtime;
	uint16_t createdate;
	uint16_t accessdate;
	uint16_t clusterhigh;

	uint16_t modifiedtime;
	uint16_t modifieddate;
	uint16_t clusterlow;
	uint32_t filesize;
} __attribute__((packed)) fat32_directory_entry;
