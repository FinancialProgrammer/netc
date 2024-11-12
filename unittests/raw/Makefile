CC = g++ -x c++ # gcc
LIBS = -lnetc

all: connect.out connect.outi rw.out rw.outi hostname.out hostname.outi hostnamerw.out hostnamerw.outi

%.outi: %.ci
	${CC} -x c -o $@ $<

%.out: %.c
	${CC} -o $@ $< ${LIBS}

clean:
	rm *.out *.outi