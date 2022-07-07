SOURCES = $(shell find "../src/" -maxdepth 1 -name "*.c" -type f)

IDPL="SDL"
SOURCES += $(shell find "../src/sdl" -name "*.c" -type f)

all:
	gcc test.c $(SOURCES) -g -o test \
	-I /home/robbe/Projects/yanix/libs/libgraphic/include -DIDPL_$(IDPL) -lSDL2 -lSDL2_gfx -I/usr/include/SDL2