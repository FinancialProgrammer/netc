CC = g++ -x c++ # gcc
LIBS = -lnetc

all: connect.out connect.ssl.out server.out server.ssl.out serverudp.out connectudp.out poll.out poll.ssl.out massconnect.out massconnect.ssl.out

%.ssl.out: %.c
	${CC} -x c -o $@ $< -D_USEOPENSSL ${LIBS}

%.out: %.c
	${CC} -o $@ $< ${LIBS}

clean:
	rm *.out