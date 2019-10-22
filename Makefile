# makefile for my kernel, i would give it a name but i don't know any good ones 
# so for now it's just Robbe's kernel :D

#C_SOURCES = $(wildcard config/*.c cpu/*.c drivers/*/*.c drivers/*/*/*.c kernel/*.c kernel/*/*.c libc/*.c libc/*/*.c gui/*.c)
#HEADERS = $(wildcard config/*.h cpu/*.h drivers/*/*.h drivers/*/*/*.c kernel/*.h kernel/*/*.h libc/*.h libc/*/*.h gui/*.h)
C_SOURCES = $(shell find ./ -type f -name '*.c' ! -path "./userspace/*" ! -path "./.testing*")
HEADERS   = $(shell find ./ -type f -name '*.h' ! -path "./userspace/*" ! -path "./.testing*")

OBJECTS = ${C_SOURCES:.c=.o asm/interrupt.o asm/gdtasm.o asm/taskingasm.o}


#FLAGS = -g -m32 -fno-pie -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror

# is -O2 compatible but is turned off for now because i wan't to be certain that the code just runs fine first (bugs come later)
FLAGS = -g -Wall -Wextra -Werror -ffreestanding -I ./ -I ./include

QEMU_FLAGS = -m 512M -device isa-debug-exit,iobase=0xf4,iosize=0x04 -hda os-image.bin -no-reboot
#			 -net nic,model=rtl8139 -no-kvm-irqchip
QEMU_NETWORK_FLAGS = -netdev user,id=u1,hostfwd=tcp::5555-:5454 -device rtl8139,netdev=u1 -object filter-dump,id=f1,netdev=u1,file=networkdump.dat

#autorun:
#	make run



os-image.bin: boot/bootloader.bin ramdisk.iso kernel.bin
	cat $^ > os-image.bin

testing:
	make clean
	make _testing

_testing: os-image.bin kernel.elf
	qemu-system-x86_64 -s -d guest_errors -m 512M -hda os-image.bin -no-reboot &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

ramdisk.iso:
	fallocate -l 384K ramdisk.iso
	mkfs.ext2 ramdisk.iso
	#python3 tools/buildramdisk.py
	
padding:
	fallocate -l 64K padding
	python3 tools/buildramdisk.py 65536

kernel.bin: boot/enter_kernel.o ${OBJECTS}
	/usr/share/crosscompiler/bin/bin/i686-elf-ld -T linker.ld -m elf_i386 -o $@ -Ttext 0x70000 $^ --oformat binary
	

kernel.elf: boot/enter_kernel.o ${OBJECTS}
	/usr/share/crosscompiler/bin/bin/i686-elf-ld -T linker.ld -m elf_i386 -o $@ -Ttext 0x70000 $^ 

run: os-image.bin
	qemu-system-x86_64 ${QEMU_FLAGS} ${QEMU_NETWORK_FLAGS} # -enable-kvm
	# this will start qemu with 2 devices (floppies) one for the kernel and one just as read write disk

run_no_network: os-image.bin
	qemu-system-x86_64 ${QEMU_FLAGS} -enable-kvm

debug: os-image.bin kernel.elf
	qemu-system-x86_64 -s -d guest_errors ${QEMU_FLAGS} ${QEMU_NETWORK_FLAGS} -enable-kvm &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.c ${HEADERS}
	/usr/share/crosscompiler/bin/bin/i686-elf-gcc ${FLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@


monitor_network: os-image.bin
	qemu-system-x86_64 ${QEMU_FLAGS} 
	tshark -r networkdump.dat 

mount_ramdisk: ramdisk.iso
	mount -o loop ./ramdisk.iso /media/iso
	gnome-terminal --working-directory=/media/iso 	# starts a shell in the ramdisk (for convenience)

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

cc_env:
	# starting cross compiling environment 
	gnome-terminal -x bash -c 'export PATH="/usr/share/crosscompiler/hosted/gcc/bin/:$$PATH"; export PATH="/usr/share/crosscompiler/hosted/binutils/i686-yanix/bin/:$$PATH"; export PATH="/usr/share/crosscompiler/hosted/binutils/bin/:$$PATH"; exec bash'
