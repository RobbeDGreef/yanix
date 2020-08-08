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
LOOPD_ROOTFS		= /dev/loop6
LOOPD_DISK			= /dev/loop5
ROOTFS 				= ./rootfs
USER 				= robbe 

# Easier in sublime
# CC = /usr/share/crosscompiler/bin/i386-elf-gcc
# LD = /usr/share/crosscompiler/bin/i386-elf-ld
CC = $(PWD)/toolchain/bin/bin/i686-yanix-gcc
LD = $(PWD)/toolchain/bin/bin/i686-yanix-ld

GDB = gdb
NASM = nasm
QEMU = qemu-system-x86_64
QEMU_DEBUG_FLAGS = -d guest_errors,cpu_reset
QEMU_FLAGS = $(QEMU_DEBUG_FLAGS) -m 512M 
QEMU_FLAGS += -device isa-debug-exit,iobase=0xf4,iosize=0x04 
QEMU_FLAGS += -netdev user,id=u1,hostfwd=tcp::5555-:5454 -device rtl8139,netdev=u1
QEMU_FLAGS += -object filter-dump,id=f1,netdev=u1,file=networkdump.dat 
QEMU_FLAGS += -serial pipe:/serial_output.out 
QEMU_FLAGS += -no-reboot #-enable-kvm
QEMU_FLAGS += -d int,cpu_reset -vga vmware
# Kvm actually slow the os down on some parts (like the ata driver)

all:
	rm -rf kernel.bin kernel.elf
	$(MAKE) install

install: kernel.bin kernel.elf
	cp kernel.bin rootfs/kernel
	cp kernel.elf rootfs/kernel.elf
	# sync up the disks
	sync
	
	#$(MAKE) install_bootloader

install_bootloader:
	$(MAKE) -C ./bootloader/ BOOTDISK=$(PWD)/$(DISKNAME) PREFIX=$(PWD)/bootloader KERNEL_ENTRY=$(KERNEL_ENTRY_POINT)

bootloader:
	# Building squat
	$(MAKE) -C ./bootloader/ build BOOTDISK=$(PWD)/$(DISKNAME) PREFIX=$(PWD)/bootloader KERNEL_ENTRY=$(KERNEL_ENTRY_POINT)

grub:
	# Using grub for now, until i find the mental strength to rewrite the bootloader lol
	sudo grub-install --root-directory=/home/robbe/Projects/yanix/rootfs --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop5 --target=i386-pc

clean:
	rm -rf kernel.bin kernel.elf $(OUTPUT_NAME)
	$(MAKE) -C ./kernel/ clean ARCH=$(ARCH) PREFIX=$(PWD) CC=$(CC) LD=$(LD) NASM=$(NASM) KERNEL_ENTRY_POINT=$(KERNEL_ENTRY_POINT)
	$(MAKE) -C ./bootloader/ clean PREFIX=$(PWD)/bootloader

run: kernel.bin maindisk.iso
	@echo "Serial output: "
	cat /dev/null > /tmp/qemu_dump.txt
	cat /serial_output.out &
	$(QEMU) ${QEMU_FLAGS} -hda $(DISKNAME) > /tmp/qemu_dump.txt 2>&1

debug: kernel.bin maindisk.iso kernel.elf
	$(QEMU) -s ${QEMU_FLAGS} -hda $(DISKNAME) > /tmp/qemu_dump.txt 2>&1 &
	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file kernel.elf" -ex "breakpoint DEBUGGER_ENTRY" -ex "directory /home/robbe/Projects/yanix/kernel" -ex "set disassembly-flavor intel"

#### Building ####

rebuild_sysroot:
	rm sysroot/usr -rf
	sh tools/rebuild_sysroot.sh

build_sysroot:
	rm sysroot/usr -rf
	sh tools/build_sysroot.sh

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
	
	mount $(LOOPD_ROOTFS) $(ROOTFS)
	
	mkdir $(ROOTFS)/bin
	mkdir $(ROOTFS)/sbin
	mkdir $(ROOTFS)/dev
	mkdir $(ROOTFS)/etc

	cp sysroot/usr $(ROOTFS)/usr -r

	umount $(ROOTFS)
	losetup -d $(LOOPD_ROOTFS)

### misc

mount_ramdisk: ramdisk.iso
	mount -o loop ./ramdisk.iso ./initrd	

unmount_ramdisk: ramdisk.iso
	umount ./initrd

mount_disk: $(DISKNAME)
	losetup $(LOOPD_DISK) $(DISKNAME) 
	losetup $(LOOPD_ROOTFS) $(DISKNAME) -o 1048576

	mount $(LOOPD_ROOTFS) $(ROOTFS)

unmount_disk:
	umount rootfs

	losetup -d $(LOOPD_ROOTFS)
	losetup -d $(LOOPD_DISK)

backup:
	make clean
	python3 tools/backup_os.py

cloc:
	cloc --exclude-dir=ports,rootfs,sysroot,sysroot_new,sysroot_old,toolchain,system,.vscode .

check:
	cppcheck --enable=all kernel -j 8 > /dev/null