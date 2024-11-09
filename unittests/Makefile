CC = gcc
LIBS = -lnetc

all: connect.out connect.outi rw.out rw.outi

%.outi: %.ci
	${CC} -x c -o $@ $<

%.out: %.c
	${CC} -o $@ $< ${LIBS}

clean:
	rm *.out