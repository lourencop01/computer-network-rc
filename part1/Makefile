CC = g++
CFLAGS = -Wall -Wextra -std=c++11

.PHONY: all clean

all: clean auction_server user_program

auction_server: as.o aux.o
	$(CC) $(CFLAGS) -o as as.o aux.o

user_program: user.o aux.o
	$(CC) $(CFLAGS) -o user user.o aux.o

as.o: as.cpp headers.h
	$(CC) $(CFLAGS) -c -o as.o as.cpp

user.o: user.cpp headers.h
	$(CC) $(CFLAGS) -c -o user.o user.cpp

aux.o: aux.cpp headers.h
	$(CC) $(CFLAGS) -c -o aux.o aux.cpp

clean:
	rm -f as user *.o

cleaner:
	rm -f as user *.o USERS/*/*.txt AUCTIONS/*/*.txt
	rm -r USERS/* AUCTIONS/*
