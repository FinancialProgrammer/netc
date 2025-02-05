

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

libnetc.so: ${OBJECTS} openssl/libssl.a openssl/libcrypto.a
	${CC} -shared -o $@ $^ ${LIBS} ${ARGS} openssl/libssl.a openssl/libcrypto.a

libnetc.a: ${OBJECTS} openssl/libssl.a openssl/libcrypto.a
	ar rcs $@ ${OBJECTS} openssl/libssl.a openssl/libcrypto.a

linux_install: ${INCLS} libnetc.so
	sudo cp -r include/* /usr/local/include/
	sudo cp libnetc.so /usr/local/lib/

clean:
	rm src/*.o src/raw/*.o src/ssl/*.o

cleanlib:
	rm libnetc.so libnetc.a