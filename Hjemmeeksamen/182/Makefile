CFLAGS = -Wall -std=gnu99 -g -o
CC = gcc
all: Server Client


Client: 
	$(CC) $(CFLAGS) Client Client.c protocols.c
	
Server: 
	$(CC) $(CFLAGS) Server Server.c protocols.c
	
clean: 
	rm Server Client
