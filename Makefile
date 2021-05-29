CC = gcc
CFLAGS = -O5 -s

all: price-server counter pusher puller

.PHONY: clean

clean:
	rm price-server counter pusher puller

##
# Executables.
##

price-server: price-server.c signals.c sockets.c pass-fd.c
	$(CC) $(CFLAGS) -o price-server price-server.c signals.c sockets.c pass-fd.c

counter: counter.c signals.c
	$(CC) $(CFLAGS) -o counter counter.c signals.c

pusher: pusher.c
	$(CC) $(CFLAGS) -o pusher pusher.c

puller: puller.c
	$(CC) $(CFLAGS) -o puller puller.c
