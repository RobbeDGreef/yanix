# makefile for my kernel, i would give it a name but i don't know any good ones 
# so for now it's just Robbe's kernel :D

#C_SOURCES = $(wildcard config/*.c cpu/*.c drivers/*/*.c drivers/*/*/*.c kernel/*.c kernel/*/*.c libc/*.c libc/*/*.c gui/*.c)
#HEADERS = $(wildcard config/*.h cpu/*.h drivers/*/*.h drivers/*/*/*.c kernel/*.h kernel/*/*.h libc/*.h libc/*/*.h gui/*.h)
C_SOURCES = $(shell find ./ -type f -name '*.c')
HEADERS   = $(shell find ./ -type f -name '*.h')

OBJECTS = ${C_SOURCES:.c=.o asm/interrupt.o asm/gdtasm.o asm/taskingasm.o}
	

#FLAGS = -g -m32 -fno-pie -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror

# is -O2 compatible but is turned off for now because i wan't to be certain that the code just runs fine first (bugs come later)
FLAGS = -g -Wall -Wextra -Werror -ffreestanding -I ./ -I ./include


os-image.bin: boot/bootsector.bin ramdisk.iso kernel.bin
	cat $^ > os-image.bin

ramdisk.iso:
	fallocate -l 64K ramdisk.iso
	mkfs.ext2 ramdisk.iso
	#python3 tools/buildramdisk.py
	
padding:
	fallocate -l 64K padding
	python3 tools/buildramdisk.py 65536

kernel.bin: boot/enter_kernel.o ${OBJECTS}
	/usr/share/crosscompiler/bin/i386-elf-ld -T linker.ld -m elf_i386 -o $@ -Ttext 0x20000 $^ --oformat binary
	

kernel.elf: boot/enter_kernel.o ${OBJECTS}
	/usr/share/crosscompiler/bin/i386-elf-ld -T linker.ld -m elf_i386 -o $@ -Ttext 0x20000 $^ 
	

debug: os-image.bin kernel.elf
	qemu-system-x86_64 -s -d guest_errors -fda os-image.bin -no-reboot -enable-kvm &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.c ${HEADERS}
	/usr/share/crosscompiler/bin/i386-elf-gcc ${FLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

run: os-image.bin
	qemu-system-x86_64 -m 512M -device isa-debug-exit,iobase=0xf4,iosize=0x04 -drive file=tools/test.bin,if=floppy,index=1 -fda os-image.bin -no-reboot -enable-kvm 
	# this will start qemu with 2 devices (floppies) one for the kernel and one just as read write disk

mount_ramdisk: ramdisk.iso
	mount -o loop ./ramdisk.iso /media/iso

umount_ramdisk:
	umount /media/iso

clean_ramdisk:
	rm ramdisk.iso 

test_ramdisk:
	echo "Hello world!" > /media/iso/helloworldfile.txt

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf padding
	rm -rf boot/*.o boot/*.bin ${OBJECTS}

backup:
	make clean
	python3 tools/backup_os.py
	
linecount:
	python3 tools/linecounter.py