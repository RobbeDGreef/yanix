##
## @defgroup   BOOTLOADER Makefile
##
## @brief      This file implements the bootlaoder Makefile.
##
## @author     Robbe De Greef
## @date       2019
##

# Prefix should be set by caller
ifndef PREFIX
	$(error PREFIX was not set)
endif

# Check if a specific nasm binairy was specified
ifndef NASM
	NASM = nasm
endif 

# Define the nasm flags
NASM_FLAGS = -f bin

# The main makefile build
all:
	$(info [INFO] Building bootloader.bin)
	# Compile the binary file
	$(NASM) bootloader.asm ${NASM_FLAGS} -o $(PREFIX)/bootloader.bin

