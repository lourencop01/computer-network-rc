
CC = g++
CFLAGS = -Wall -Wextra -std=c++11

all: clean as client

as: as.cpp
	$(CC) $(CFLAGS) -o as as.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	rm -f as client
