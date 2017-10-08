
CFLAGS= -g -Wall -pedantic 
CC=gcc

all: client_server_1

client_server_1: client_server_1.c  mplib3.c leQueue.c leQueue.h mplib3.h
	$(CC) $(CFLAGS) -lpthread -o client_server_1 client_server_1.c mplib3.c leQueue.c

clean:
	rm -f *.o client_server_1

leak:	client_server_1
	valgrind --tool=memcheck --leak-check=yes ./client_server_1 4