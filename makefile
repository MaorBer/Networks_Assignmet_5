.PHONY = clean 


all: Sniffer Spoofer Snoofer

Sniffer: Sniffer.c
	gcc -Wall Sniffer.c -o Sniffer  -lpcap
Spoofer: Spoofer.c
	gcc -Wall Spoofer.c -o Spoofer 

# Snoofer: Snoofer.c
# 	gcc Snoofer.c -o Snoofer -Wall -lpcap

clean:
	rm -f *.o Sniffer Spoofer Snoofer