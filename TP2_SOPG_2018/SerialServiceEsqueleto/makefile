# $@ es una variable que contiene el target que se esta ejecutando
# $^ es una variable que contiene todas las dependencias de la regla ejecutada
# .PHONY es para que no haya conflicto si un archivo se llama "clean"

CC=gcc
CFLAGS=-Wall -pthread
DEPS=rs232.h SerialManager.h ClientData.h serie.h server.h manejoThreads.h
APP=serialService
src=$(wildcard *.c)
obj=$(src:.c=.o)

$(APP): $(obj)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -rf *.o  $(APP)
