CC = gcc
CFLAGS = -O3 -s

all: price-server counter pusher puller

.PHONY: clean

clean:
	rm price-server counter pusher puller

##
# Executables.
##

price-server: price-server.c signals.c sockets.c pass-fd.c
	$(CC) $(CFLAGS) -o price-server price-server.c signals.c sockets.c pass-fd.c

counter: counter.c signals.c sockets.c
	$(CC) $(CFLAGS) -o counter counter.c signals.c sockets.c

pusher: pusher.c signals.c
	$(CC) $(CFLAGS) -o pusher pusher.c signals.c

puller: puller.c signals.c sockets.c
	$(CC) $(CFLAGS) -o puller puller.c signals.c sockets.c
