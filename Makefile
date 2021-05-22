CC = gcc

all: counter pusher puller

counter: counter.c
	$(CC) -o counter counter.c

pusher: pusher.c
	mkdir -p $(BIN)
	$(CC) -o pusher pusher.c

puller: puller.c
	mkdir -p $(BIN)
	$(CC) -o puller puller.c
