##
## @defgroup   MAKEFILE Makefile
##
## @brief      This file implements the main os Makefile.
##
## @author     Robbe De Greef
## @date       2019
##

.PHONY = all run debug mount_ramdisk unmount_ramdisk clean

KERNEL_ENTRY_POINT 	= 0x70000
OUTPUT_NAME 		= os-image.bin
RAMDISK_SIZE		= 384K
ARCH 				= i386


# Easier in sublime
CC = /usr/share/crosscompiler/bin/i386-elf-gcc
LD = /usr/share/crosscompiler/bin/i386-elf-ld
GDB = gdb
NASM = nasm
QEMU = qemu-system-x86_64
QEMU_FLAGS = -m 512M -device isa-debug-exit,iobase=0xf4,iosize=0x04 -no-reboot -netdev user,id=u1,hostfwd=tcp::5555-:5454 -device rtl8139,netdev=u1 -object filter-dump,id=f1,netdev=u1,file=networkdump.dat

all:
	rm -rf kernel.bin kernel.elf bootloader.bin
	make $(OUTPUT_NAME)

$(OUTPUT_NAME): bootloader.bin ramdisk.iso kernel.bin 
	cat $^ > $(OUTPUT_NAME)


clean:
	rm -rf kernel.bin kernel.elf bootloader.bin $(OUTPUT_NAME)
	$(MAKE) -C ./kernel/ clean ARCH=$(ARCH) PREFIX=$(PWD) CC=$(CC) LD=$(LD) NASM=$(NASM) KERNEL_ENTRY_POINT=$(KERNEL_ENTRY_POINT)

run: $(OUTPUT_NAME)
	
	$(QEMU) ${QEMU_FLAGS} -hda $(OUTPUT_NAME) -hdb kernel.bin

debug: $(OUTPUT_NAME) kernel.elf
	$(QEMU) -s -d guest_errors ${QEMU_FLAGS} -hda $(OUTPUT_NAME) &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

mount_ramdisk: ramdisk.iso
	mount -o loop ./ramdisk.iso ./initrd	

unmount_ramdisk: ramdisk.iso
	umount ./initrd

backup:
	make clean
	python3 tools/backup_os.py

#### Building ####

kernel.bin:
	$(info [INFO] Building kernel)
	$(MAKE) -C ./kernel/ ARCH=$(ARCH) PREFIX=$(PWD) CC=$(CC) LD=$(LD) NASM=$(NASM) KERNEL_ENTRY_POINT=$(KERNEL_ENTRY_POINT)

kernel.elf:
	$(info [INFO] Building kernel elf image)
	$(MAKE) -C ./kernel ARCH=$(ARCH) PREFIX=$(PWD) CC=$(CC) LD=$(LD) NASM=$(NASM) KERNEL_ENTRY_POINT=$(KERNEL_ENTRY_POINT) KERNEL_ELF=1

bootloader.bin:
	$(info [INFO] Building bootloader)
	$(MAKE) -C ./bootloader PREFIX=$(PWD) NASM=$(NASM)

ramdisk.iso:
	fallocate -l RAMDISK_SIZE ramdisk.iso
	mkfs.ext2 ramdisk.iso