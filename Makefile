

CC = gcc
LIBS = 
INCL = -I./include/
ARGS = -O3

INCLS = $(shell find ./include -name '*.h')

SRCS = $(shell find ./src -name '*.c')
OBJECTS = ${SRCS:.c=.o}

all: libnetc.so libnetc.a

%.o: %.c ${INCLS}
	${CC} -c -o $@ $< ${INCL} ${ARGS}

libnetc.so: ${OBJECTS}
	${CC} -shared -o $@ $^ ${LIBS} ${ARGS}

libnetc.a: ${OBJECTS}
	ar rcs $@ ${OBJECTS}
#	${CC} -static -o $@ $^ ${LIBS} ${ARGS}

install: ${INCLS}
	sudo cp -r include/* /usr/local/include/
	sudo cp libnetc.* /usr/local/lib/

