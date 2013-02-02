#
# Copyright (c) 2005-2006, Wei Mingzhi. All rights reserved.
#
# Use, redistributions and modifications of this file is
# unrestricted provided the above copyright notice is
# preserved.
#

OBJ = \
	../../src/compress.o fntgen.o

CC = gcc
CXX = g++

TARGET = fntgen

BASEFLAGS = -s -O3 -I../../src

CFLAGS = ${BASEFLAGS} `sdl-config --cflags`
LDFLAGS = ${BASEFLAGS} `sdl-config --libs` -lSDL_ttf

all: ${TARGET}

${TARGET}: ${OBJ}
	${CXX} ${LDFLAGS} -o ${TARGET} ${OBJ}

clean:
	rm -f *.o ${TARGET}

distclean:
	rm -f *.o ${TARGET}

%.o:    %.cpp
	${CXX} ${CFLAGS} -c $< -o $@

%.o:    %.c
	${CC} ${CFLAGS} -c $< -o $@

