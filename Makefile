CC = gcc

all: counter pusher puller

.PHONY: clean

clean:
	rm counter pusher puller

##
# Executables.
##

counter: counter.c
	$(CC) -o counter counter.c

pusher: pusher.c
	$(CC) -o pusher pusher.c

puller: puller.c
	$(CC) -o puller puller.c
