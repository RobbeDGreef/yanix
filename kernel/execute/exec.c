#include <drivers/vfs/vfs.h>
#include <stdint.h>
#include <stdarg.h>
#include <kernel/execute/elf.h>
#include <mm/heap.h>
#include <errno.h>

int execve(const char *filename, char *const argv[], char *const envp[])
{
	(void) (envp);
	struct file *file = vfs_open(filename, 0, 0);	// open file for reading and with no flags
	if (file == 0) {
		return -1;
	}
	
	void *buf = (void*) kmalloc(file->filesize);

	int size = vfs_read(file, buf, file->filesize);
	if (size == -1){
		return -1;
	}
	
	uint32_t ret = load_elf_into_mem(buf);

	if (ret == 0) {
		return -1;
	}
	
	size_t amount = 0;
	while (argv[amount] != 0) {
		amount++;
	}
	amount += 2;

	int (*program)(int amount, char *const argv[]) = (int (*) (int amount, char *const argv[])) ret;
	program(amount, argv);
	return 0; 
}
