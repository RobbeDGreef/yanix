NASM = nasm
NASM_FLAGS = -f bin

KERNEL_FILE = ../rootfs/kernel

GET_FS_SIZE = du -b first_stage.bin | cut -d "	" -f1
GET_SS_SIZE = du -b second_stage.bin | cut -d "	" -f1
GET_KERNEL_INODE = ls -i $(KERNEL_FILE) | cut -d " " -f1

SECOND_STAGE_ENTRY = 0x9000

all: first_stage.bin second_stage.bin

ifndef BOOTDISK
	$(error BOOTDISK not defined)
endif

	# Install the bootloader first stage
	dd if=first_stage.bin of=$(BOOTDISK) bs=1 seek=0 count=$(shell $(GET_FS_SIZE)) conv=notrunc

	# And install the second stage
	dd if=second_stage.bin of=$(BOOTDISK) bs=1 seek=512 count=$(shell $(GET_SS_SIZE)) conv=notrunc


first_stage.bin: second_stage.bin
	# @TODO: add in automatic defines (like size of second stage etc)
	$(NASM) stage1/mbr.asm ${NASM_FLAGS} -o first_stage.bin

second_stage.bin:
	$(MAKE) -C ./stage2 PREFIX=$(PREFIX) NASM=$(NASM) SECOND_STAGE_ENTRY=$(SECOND_STAGE_ENTRY) LD=$(LD) KERNEL_ENTRY=$(KERNEL_ENTRY) KERNEL_INODE=$(shell $(GET_KERNEL_INODE))

second_stage.elf:
	$(MAKE) -C ./stage2 $(PREFIX)/second_stage.elf PREFIX=$(PREFIX) NASM=$(NASM) SECOND_STAGE_ENTRY=$(SECOND_STAGE_ENTRY) LD=$(LD) KERNEL_ENTRY=$(KERNEL_ENTRY) KERNEL_INODE=$(shell $(GET_KERNEL_INODE))

clean:
	rm -rf first_stage.bin second_stage.bin
	$(MAKE) -C ./stage2 clean PREFIX=$(PREFIX)

debug: all second_stage.elf
	qemu-system-i386 -hda ../maindisk.iso -s -d guest_errors &
	gdb -ex "target remote localhost:1234" -ex "set architecture i8086" -ex "symbol-file second_stage.elf"
