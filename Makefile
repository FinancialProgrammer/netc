

CC = gcc
LIBS = -lglfw -lvulkan
INCL = -I./include/
ARGS = -fPIC

INCLS = $(shell find ./include -name '*.h')

all: install

install: ${INCLS}
	sudo cp -r include/* /usr/local/include/

