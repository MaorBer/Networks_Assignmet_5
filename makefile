.PHONY = clean all


all: Sniffer Spoofer Snoofer

Sniffer: Sniffer.c
	gcc -Wall -g Sniffer.c -o Sniffer  -lpcap
Spoofer: Spoofer.c
	gcc -Wall -g Spoofer.c -o Spoofer 

Snoofer: Snoofer.c
	gcc -Wall -g Snoofer.c -o Snoofer  -lpcap

clean:
	rm -f *.o Sniffer Spoofer Snoofer