#!/bin/bash

cd /home/robbe/Projects/yanix/toolchain/newlib/newlib-3.2.0/newlib/libc/sys/yanix
cp /home/robbe/Projects/yanix/kernel/include/yanix/* ./include -r

cd /home/robbe/Projects/yanix/toolchain/newlib/build
export PATH="/home/robbe/Projects/yanix/toolchain/bin/bin:$PATH"
make all
rm /home/robbe/Projects/yanix/sysroot/usr/* -fr
make install DESTDIR=/home/robbe/Projects/yanix/sysroot
mv /home/robbe/Projects/yanix/sysroot/usr/i686-yanix/* /home/robbe/Projects/yanix/sysroot/usr/
