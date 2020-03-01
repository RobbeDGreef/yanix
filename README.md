# yanix
Just a UNIX-like kernel build on caffeine and 
sleep deprivation. I call it yanix (**Y**et **A**nother u**NIX** clone) The goal is to make it as **POSIX**
compliant as possible and self sustaining.

This project is a hobby os so this is not perfect,
I am currently trying to make the codebase as clean as possible
so others can learn from this os and build their own
but as of now some parts especially the older parts of the os
are not the prettiest / most well documented code ever.

You are welcome to make any pull requests and help build this os
further. 

If you are starting out I really recommend the osdev.org wiki 
as a lot of the code here is build on the information found
on that wiki. For more helpfull links check the bottom of this page

## Currently supported features:
### CPU-based Features:
#### Supported Architectures:
> - [X] x86
> - [ ] x86-64

- [X] Multitasking
- [ ] Multiprocessing
- [X] User mode

### Kernel Features:
- [X] Monolithic kernel
- [X] Preemptible kernel
- [X] Virtual filesystem
- [X] Execve system call (still in development)
- [X] Elf exec driver
- [X] Memory management
- [X] Ramdisk support
- [ ] Moduling system

### Supported Filesystems:
- [X] ext2
- [ ] fat32
- [ ] ext3
- [ ] ext4

### Supported Devices & Drivers:
- [X] i8042         (PS/2 Controller)
- [X] Ramdisk       (memory mapped storage device)
- [X] ps2 mouse 
- [X] ps2 keyboard
- [X] CMOS driver
- [X] PCI driver
- [ ] Floppy driver (not sure if it will be supported)
- [ ] ata
- [-] networking

### Supported Video Interfaces:
- [X] VESA 3.0 graphics
- [X] Standard video memory

### System Features:
- [X] Home made bootloader
 
## Features/stuff coming in next updates:
- Userspace shell
- Time system
- Networking
- ata driver
- Code cleaning because it is a mess at some places
- Documenting all the functions 
- Redo the ext2 driver because it is very messy and incomplete

## Latest commit includes:
- Various bug fixes
- A better execve system call.
- A lot of new system calls (most of them are stubbed for now)
- A libc, I ported newlib to yanix and I'm now working on a os specific toolchain to start userspace development and create a shell afterwards.
- FIGlet port!

## Ported software
Right now we are working hard on porting other unix software to this operating system.
The first port ever completed is FIGlet and it runs perfectly on yanix!

	__   __          _        _                                                   
	\ \ / /_ _ _ __ (_)_  __ (_)___    __ ___      _____  ___  ___  _ __ ___   ___ 
	 \ V / _` | '_ \| \ \/ / | / __|  / _` \ \ /\ / / _ \/ __|/ _ \| '_ ` _ \ / _ \
	  | | (_| | | | | |>  <  | \__ \ | (_| |\ V  V /  __/\__ \ (_) | | | | | |  __/
	  |_|\__,_|_| |_|_/_/\_\ |_|___/  \__,_| \_/\_/ \___||___/\___/|_| |_| |_|\___|

- [x] FIGlet
- [ ] Bash
- [ ] GCC
- [ ] TCC
- [ ] lua

## Getting it to work on your machine:

In order to boot the kernel you will need qemu and a copy of our os-image.bin:

#### to install qemu for ubuntu/debian users:

run the following commands to install qemu
- apt-get update
- apt-get install qemu-system-i386

#### running yanix:

to just run yanix you could use this command
- qemu-system-i386 -m 512M -hda os-image.bin

to run it with a ethernet card:
- qemu-system-i386 -m 512M -device rtl8139 -hda os-image.bin

#### Building yanix
to build yanix you will need a crosscompiler
a way to easily build one will show here soon.


## Links

A big thanks to anyone that helped on one of the articles found bellow
without those people this project would not have been possible.

#### osdev wiki and forum:
- https://wiki.osdev.org  && https://forum.osdev.org

#### The James Molloy kernel development tutorials:

A few notes about these tutorials:
I personally would not recommend these articles if you just are starting out 
as these generally do not explain the material very deeply and you are at risk
of just copying code that works and not really understanding what happens under 
the hood. The tutorials also have some bugs in them check (this page)[https://wiki.osdev.org/James_Molloy%27s_Tutorial_Known_Bugs]
this page for more information

- http://www.jamesmolloy.co.uk/tutorial_html/

#### Writing a Simple Operating System from scratch - Nick Blundell

This is a pdf that explains all the beginnings of an os and it's
bootloader, the document explains the material very good.

- http://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

#### os-tutorial - cfenollosa

This page is also very good if you are just starting out although it is mostly
based on the document by Nick Blundell that explains everything, so you might as
well use that. 

- https://github.com/cfenollosa/os-tutorial

#### The Internet

If you still struggle to understand something our want to implement something that
is not listed in any of these documents you can still search the web. Sadly enough
a lot of os development sites and articles are now down because of various reasons
but there is still a lot just lying around waiting for you to find it.
