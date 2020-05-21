##
## @defgroup   KERNEL Makefile
##
## @brief      This file implements the kernel Makefile.
##
## @author     Robbe De Greef
## @date       2019
##


# Check if architecture is defined
ifndef ARCH 
	$(error ARCH was not set)
endif

# Check if the prefix is defined
ifndef PREFIX 
	$(error PREFIX was not set)
endif

# Check if a kernel entry point is given
ifndef KERNEL_ENTRY_POINT
	$(error KERNEL_ENTRY_POINT was not set)
endif

# No crosscompiler ?
ifndef LD
	$(error LD was not set, crosscompiler needed)
endif

# No crosscompiler ?
ifndef CC
	$(error CC was not set, crosscompiler needed)
endif

ifndef NASM
	NASM = nasm 
endif

# Finding all the source files
C_SOURCES = $(shell find $(PWD)/kernel -type f -name '*.c' ! -path "$(PWD)/.*/*" ! -path "./arch/*")
HEADERS   = $(shell find $(PWD)/kernel -type f -name '*.h' ! -path "$(PWD)/.*/*" ! -path "./arch/*")

# Adding correct arch folder
C_SOURCES += $(shell find "$(PWD)/kernel/arch/$(ARCH)/" -type f -name "*.c")
HEADER    += $(shell find "$(PWD)/kernel/arch/$(ARCH)/" -type f -name "*.h")
ASMFILES   = $(shell find "$(PWD)/kernel/arch/$(ARCH)/" -type f -name "*.asm" ! -path $(PWD)/kernel/arch/$(ARCH)/boot/)	# Anything in boot should be used in the makefile here

# Defining the objects to build
OBJECTS = ${C_SOURCES:.c=.o}
OBJECTS += ${ASMFILES:.asm=.o}

# Custom flags can be used otherwise this will be used
ifndef CC_FLAGS
	CC_FLAGS = -g -O0 -Wall -Wextra -Werror -ffreestanding -I $(PWD)/kernel/include -I $(PWD)/kernel/arch/$(ARCH)/include -Wno-unused-variable -Wno-unused-function
endif

# Custom flags can be used otherwise this will be used
ifndef LD_FLAGS
	LD_FLAGS = -m elf_$(ARCH)
endif

.PHONY = all clean


all: $(PWD)/kernel/arch/$(ARCH)/boot/enter_kernel.o ${OBJECTS}
	$(info [INFO] Linking kernel)
	$(LD) -T linker.ld ${LD_FLAGS} -o $(PREFIX)/kernel.bin -Ttext $(KERNEL_ENTRY_POINT) $^ --oformat binary


ifdef KERNEL_ELF
	# Kernel.elf was also requested
	$(LD) -T linker.ld ${LD_FLAGS} -o $(PREFIX)/kernel.elf -Ttext $(KERNEL_ENTRY_POINT) $^ 
endif

%.o: %.c ${HEADERS}
	$(CC) ${CC_FLAGS} -c $< -o $@

%.o: %.asm
	$(NASM) $< -f elf -o $@

%.bin: %.asm
	$(NASM) $< -f bin -o $@

clean:
	$(info [INFO] cleaning kernel depencies)
	rm -rf ${OBJECTS}
