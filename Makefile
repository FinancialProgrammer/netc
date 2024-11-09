

CC = gcc
LIBS = 
INCL = -I./include/
ARGS = -fPIC -O3 -Ofast

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

install: ${INCLS} libnetc.so
	sudo cp -r include/* /usr/local/include/
	sudo cp libnetc.so /usr/local/lib/

clean:
	rm src/*.o libnetc.so libnetc.a