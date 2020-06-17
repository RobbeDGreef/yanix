#!/bin/bash

cd /home/robbe/Projects/yanix/toolchain/newlib/newlib-3.2.0/newlib/libc/sys/yanix
cp /home/robbe/Projects/yanix/kernel/include/yanix/* ./include -r
/home/robbe/Projects/yanix/toolchain/newlib/utils/bin/bin/aclocal -I ../../..
/home/robbe/Projects/yanix/toolchain/newlib/utils/bin/bin/automake --cygnus Makefile
cd ..
/home/robbe/Projects/yanix/toolchain/newlib/utils/bin/bin/autoconf
cd yanix
/home/robbe/Projects/yanix/toolchain/newlib/utils/bin/bin/autoconf
cd /home/robbe/Projects/yanix/toolchain/newlib/build
export PATH="/home/robbe/Projects/yanix/toolchain/bin/bin:$PATH"
../newlib-3.2.0/configure --prefix=/usr --target=i686-yanix
make all
rm /home/robbe/Projects/yanix/sysroot/usr/* -fr
make install DESTDIR=/home/robbe/Projects/yanix/sysroot
mv /home/robbe/Projects/yanix/sysroot/usr/i686-yanix/* /home/robbe/Projects/yanix/sysroot/usr/
