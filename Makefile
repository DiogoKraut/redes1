CC = gcc
CFLAGS = -g -Wall
HEADERS = $(wildcard *.h)
OBJECTS = cmds.o socket.o
TARGET = server client
LIBS = -lm

default: $(TARGET)
all: default clean

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


server: server.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $< -o $@

client: client.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $< -o $@


clean:
	rm -f *.o core

purge:
	rm -f *.o
	rm -f $(TARGET)