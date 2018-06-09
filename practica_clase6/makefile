CC=gcc 
CFLAGS=-Wall -pthread

all: cliente_tcp server_tcp

cliente_tcp: cliente_tcp.c
	$(CC) -o cliente_tcp cliente_tcp.c $(CFLAGS)

server_tcp: server_tcp.c
	$(CC) -o server_tcp server_tcp.c ClientData.c $(CFLAGS)

clean:
	rm -rf *.o server_tcp cliente_tcp
