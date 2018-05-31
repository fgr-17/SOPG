CC=gcc
CFLAGS=-Wall

all: reader writer

reader: reader.c
	$(CC) -o reader reader.c $(CFLAGS)

writer: writer.c
	$(CC) -o writer writer.c $(CFLAGS)

clean:
	rm -rf *.o reader writer Log.txt Sign.txt myfifo
