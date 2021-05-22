CC = gcc

all: price-server counter pusher puller

.PHONY: clean

clean:
	rm price-server counter pusher puller

##
# Executables.
##

price-server: price-server.c
	$(CC) -o price-server price-server.c

counter: counter.c signals.c
	$(CC) -o counter counter.c signals.c

pusher: pusher.c
	$(CC) -o pusher pusher.c

puller: puller.c
	$(CC) -o puller puller.c
