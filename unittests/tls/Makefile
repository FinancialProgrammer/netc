CC = gcc
LIBS = -lnetc # -lssl -lcrypto

all: connect.out connect.outi rw.out rw.outi

%.outi: %.ci
	${CC} -x c -o $@ $< ${LIBS}

%.out: %.c
	${CC} -o $@ $< ${LIBS}

clean:
	rm *.out *.outi