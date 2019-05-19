#os
Just a UNIX-like kernel that I am writing
I have no name for it yet so I just call it OS.
The goal is to make it fully **POSIX** compliant
and self sustaining but that is still pretty far off.

This project is the largest I have done so far
and is completely build by me and I am trying to
make it self sustaining as fast as possible.

I've come a long way but there is still a 
long way to go aswell.

## Currently supported features:
### CPU-based Features:
#### Supported Architechtures:
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
- [ ] ata
- [ ] networking

### Supported Video Interfaces:
- [X] VESA 3.0 graphics
- [X] Standard video memory

### System Features:
- / nothing yet, still working on executing userspace programs

## Features/stuff comming in next updates:
- Userspace shell
- Execve system call
- Time system
- Networking
- ata driver
- Code cleaning because it is a mess at some places
- Documenting all the functions 
