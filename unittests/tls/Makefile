CC = g++ -x c++ -fstack-protector-all -g
# gcc
LIBS = -lnetc # -lssl -lcrypto

all: connect.out connect.outi rw.out rw.outi hostname.out hostname.outi hostnamerw.out hostnamerw.outi

%.outi: %.ci
	${CC} -x c -o $@ $< ${LIBS}

%.out: %.c
	${CC} -o $@ $< ${LIBS}

clean:
	rm *.out *.outi