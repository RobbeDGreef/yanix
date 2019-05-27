#include <drivers/vfs/vfs.h>
#include <stdint.h>
#include <stdarg.h>
#include <kernel/execute/elf.h>
#include <mm/heap.h>
#include <errno.h>

#include <drivers/video/videoText.h>

int execve(const char *filename, char *const argv[], char *const envp[])
{

	struct file *file = vfs_open(filename, 0, 0);	// open file for reading and with no flags
	if (file == 0) {
		return -1;
	}
	
	void *buf = (void*) kmalloc(file->length);

	int size = vfs_read(file, buf, 624);
	if (size == -1){
		return -1;
	}
	uint32_t ret = load_elf_into_mem(buf);

	if (ret == 0) {
		return -1;
	}
	
	int (*program)(char *const argv[], char *const envp[]) = (int (*) (char *const argv[], char *const envp[]))ret;
	program(argv, envp);
	return 0; 
}
