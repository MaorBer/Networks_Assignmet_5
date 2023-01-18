all: Sniffer Spoofer Snoofer

# Sniffer: Sniffer.c
# 	gcc Sniffer.c -o Sniffer -Wall -lpcap
Spoofer: Spoofer.c
	gcc Spoofer.c -o Spoofer -Wall
# Snoofer: Snoofer.c
# 	gcc Snoofer.c -o Snoofer -Wall -lpcap

clean:
	rm -f *.o Sniffer Spoofer Snoofer