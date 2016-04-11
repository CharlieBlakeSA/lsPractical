CC=gcc
CFLAGS= -Wall -Wextra -std=c99 -g

all: ls-stage1 ls-stage2

ls-stage1: ls-stage1.c
	$(CC) -o $@ $^ $(CFLAGS)

ls-stage2: ls-stage2.c flags.c directory_sorting.c file_info.c ls-stage2.h
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f ls-stage1 ls-stage2
