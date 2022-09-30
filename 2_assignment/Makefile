CC=gcc 
CFLAGS=-std=c99 -Wall

all: sigtrap random vsed

vsed: vsed_0.c pcb_0.c pcb_0.h vsed_0.h
	$(CC) $(CFLAGS) vsed_0.c pcb_0.c -o vsed

random: random_aps.c
	$(CC) $(CFLAGS) random_aps.c -o random

sigtrap: sigtrap.c
	$(CC) $(CFLAGS) sigtrap.c -o sigtrap

clean:
	rm sigtrap random vsed
