C_SOURCES=$(shell find . -name "*.c" -type f)
HEADERS=$(shell find . -name "*.h" -type f)

all: winserv install

winserv:
	i686-yanix-gcc $(CFLAGS) ${C_SOURCES} -o winserv -I ./include -I ../../external

install: winserv
	mv winserv ../../rootfs/bin/ws
