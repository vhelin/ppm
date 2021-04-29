compiler = gcc

sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)

cflags += $(shell pkg-config --cflags libpng) $(shell pkg-config --cflags gtk+-2.0) -Wno-deprecated-declarations -Wno-discarded-qualifiers
lflags += $(shell pkg-config --libs   libpng) $(shell pkg-config --libs   gtk+-2.0)

out = ppm

all: $(out)

ifeq ($(OS),Windows_NT)
cflags += -DWIN32
endif

ifeq ($(PREFIX),)
PREFIX = /usr/bin
endif

install:
	cp $(out) $(PREFIX)/$(out)

$(out): $(objects)
	$(compiler) $(objects) $(lflags) -o $(out) 

%.o: %.c
	$(compiler) -c $(cflags) $< -o $@

clean:
	-@rm $(objects)
	-@rm $(out)
