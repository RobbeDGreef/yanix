C_SOURCES = $(shell find "$(PWD)" -type f -name "*.c")
OBJECTS   = ${C_SOURCES:.c=}

all: ${OBJECTS}
	mv $^ $(DESTDIR)

%: %.c
	$(CC) $< -o $@

clean:
	rm -rf ${OBJECTS}