CC=gcc
CFLAGS= -Wall -Wextra -std=c99 -g

all: ls-stage1

ls-stage1: ls-stage1.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f ls-stage1
