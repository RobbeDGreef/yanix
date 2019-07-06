#os
Just a UNIX-like kernel build on caffeine and 
sleep deprivation. I call it yanix (**Y**et **A**nother u**NIX** clone) The goal is to make it as **POSIX**
compliant as possible and self sustaining.

This project is the largest I have done so far
and is completely build by myself so it's not
from perfect.


## Currently supported features:
### CPU-based Features:
#### Supported Architectures:
> - [X] x86
> - [ ] x86-64

- [X] Multitasking
- [ ] Multithreading
- [X] User mode

### Kernel Features:
- [X] Monolithic kernel
- [X] Preemptible kernel
- [X] Virtual filesystem
- [ ] Support for loadable modules

### Supported Filesystems:
- [X] ext2
- [ ] fat32
- [ ] ext3
- [ ] ext4

### Supported Devices & Drivers:
- [X] i8042        (PS/2 Controller)
- [X] ramdisk      (memory mapped storage device)
- [X] ps2 mouse 
- [X] ps2 keyboard
- [X] CMOS driver
- [ ] ata
- [ ] networking

### Supported Video Interfaces:
- [X] VESA 3.0 graphics
- [X] Standard video memory

### System Features:
- [X] Execve system call (still very much in development)
- [X] Elf static exec driver

## Features/stuff coming in next updates:
- Userspace shell
- Time system
- Networking
- ata driver
- Code cleaning because it is a mess at some places
- Documenting all the functions 
- Redo the ext2 driver because it is very messy and incomplete

## Latest commits include:
- Various bug fixes
- A better execve system call but the environment variables are still nonexistent and it only executes static elf files.
- A lot of new system calls (most of them are stubbed for ....)
- A libc, I ported newlib to yanix and I'm now working on a os specific toolchain to start userspace development and create a shell afterwards.