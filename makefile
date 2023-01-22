.PHONY = clean all

CC = gcc
FLAGS = -Wall -g 

all: Sniffer Spoofer Snoofer Gateway

Sniffer: Sniffer.c
	$(CC) $(FLAGS) Sniffer.c -o Sniffer  -lpcap
Spoofer: Spoofer.c
	$(CC) $(FLAGS) Spoofer.c -o Spoofer 

Snoofer: Snoofer.c
	$(CC) $(FLAGS) Snoofer.c -o Snoofer  -lpcap

Gateway: Gateway.c	
	$(CC) $(FLAGS) Gateway.c -o Gateway -lpcap
clean:
	rm -f *.o Sniffer Spoofer Snoofer