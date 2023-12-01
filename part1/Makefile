CC = g++
CFLAGS = -Wall -Wextra -std=c++11

.PHONY: all clean

all: clean auction_server client_program

auction_server: as.o aux.o
	$(CC) $(CFLAGS) -o as as.o aux.o

client_program: client.o aux.o
	$(CC) $(CFLAGS) -o user client.o aux.o

as.o: as.cpp headers.h
	$(CC) $(CFLAGS) -c -o as.o as.cpp

client.o: client.cpp headers.h
	$(CC) $(CFLAGS) -c -o client.o client.cpp

aux.o: aux.cpp headers.h
	$(CC) $(CFLAGS) -c -o aux.o aux.cpp

clean:
	rm -f as user *.o
