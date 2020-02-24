##
## @defgroup   MAKEFILE Makefile
##
## @brief      This file implements the main os Makefile.
##
## @author     Robbe De Greef
## @date       2019
##

.PHONY = all run debug mount_ramdisk unmount_ramdisk clean

KERNEL_ENTRY_POINT 	= 0x100000
OUTPUT_NAME 		= os-image.bin
RAMDISK_SIZE		= 384K
ARCH 				= i386
DISK_SIZE			= 250M
DISKNAME 			= maindisk.iso
LOOPD_ROOTFS		= /dev/loop5
LOOPD_DISK			= /dev/loop4

# Easier in sublime
CC = /usr/share/crosscompiler/bin/i386-elf-gcc
LD = /usr/share/crosscompiler/bin/i386-elf-ld
GDB = gdb
NASM = nasm
QEMU = qemu-system-x86_64
QEMU_FLAGS =  -m 512M -device isa-debug-exit,iobase=0xf4,iosize=0x04 -no-reboot -netdev user,id=u1,hostfwd=tcp::5555-:5454 -device rtl8139,netdev=u1
QEMU_FLAGS += -object filter-dump,id=f1,netdev=u1,file=networkdump.dat -serial pipe:/serial_output.out

all:
	rm -rf kernel.bin kernel.elf
	$(MAKE) install

install: kernel.bin
	cp kernel.bin rootfs/kernel
	# sync up the disks
	sync
	
	$(MAKE) install_bootloader

install_bootloader:
	$(MAKE) -C ./bootloader/ BOOTDISK=$(PWD)/$(DISKNAME) PREFIX=$(PWD)/bootloader KERNEL_ENTRY=$(KERNEL_ENTRY_POINT)

clean:
	rm -rf kernel.bin kernel.elf $(OUTPUT_NAME)
	$(MAKE) -C ./kernel/ clean ARCH=$(ARCH) PREFIX=$(PWD) CC=$(CC) LD=$(LD) NASM=$(NASM) KERNEL_ENTRY_POINT=$(KERNEL_ENTRY_POINT)
	$(MAKE) -C ./bootloader/ clean PREFIX=$(PWD)/bootloader

run: kernel.bin maindisk.iso
	@echo "Serial output: "
	cat /serial_output.out &
	$(QEMU) ${QEMU_FLAGS} -hda $(DISKNAME)

debug: kernel.bin maindisk.iso kernel.elf
	$(QEMU) -s -d guest_errors ${QEMU_FLAGS} -hda $(DISKNAME) &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

mount_ramdisk: ramdisk.iso
	mount -o loop ./ramdisk.iso ./initrd	

unmount_ramdisk: ramdisk.iso
	umount ./initrd

mount_disk: $(DISKNAME)
	losetup $(LOOPD_DISK) $(DISKNAME) 
	losetup $(LOOPD_ROOTFS) $(DISKNAME) -o 1048576

	mount $(LOOPD_ROOTFS) ./rootfs

unmount_disk:
	umount rootfs

	losetup -d $(LOOPD_ROOTFS)
	losetup -d $(LOOPD_DISK)

	# just to be sure
	umount maindisk.iso

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

ramdisk.iso:
	fallocate -l $(RAMDISK_SIZE) ramdisk.iso
	mkfs.ext2 ramdisk.iso

$(DISKNAME):
	fallocate -l $(DISK_SIZE) $(DISKNAME)
	sh ./tools/create_part.sh $(DISKNAME)
	losetup $(LOOPD_ROOTFS) $(DISKNAME) -o 1048576
	mkfs.ext2 $(LOOPD_ROOTFS)
	losetup -d $(LOOPD_ROOTFS)