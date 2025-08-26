CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra

all: afd

afd: main.o
	$(CC) $(CFLAGS) -o afd main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o afd
