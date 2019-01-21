# This Makefile was altered from the Makefile for Hello GLUT.

# Directories containing C headers. Edit to match path containing freeglut headers.
INCLUDE_DIRS = -I.\include -I"C:\Program Files\Common Files\MinGW\freeglut\include"

# Directories containing import libraries. Edit to match path containing freeglut libs.
LIB_DIRS = -L"C:\Program Files\Common Files\MinGW\freeglut\lib"

# Warnings to be raised by the C compiler
WARNS = -Wall

# Names of tools to use when building
CC = gcc

# Compiler and linker flags
CFLAGS = -O3 -std=c99 -D _WIN32_IE=0x0500 -D WINVER=0x500 ${WARNS} ${INCLUDE_DIRS}
LDFLAGS = -s -lfreeglut -lglu32 -lopengl32 -Wl,--subsystem,windows ${LIB_DIRS}

# Object files to create Hello GLUT exe
OBJS = glm.o \
       gltb.o \
	   smooth.o

.PHONY: all
	   
all: smooth.exe clean

clean	:
	@del *.o

clobber	: clean
	@del *.exe

smooth.exe: ${OBJS}
	${CC} -o "$@" ${OBJS} ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

# dependencies (must come AFTER inference rules)
smooth.o	: glm.h
smooth.o    : gltb.h
glm.o		: glm.h
gltb.o		: gltb.h