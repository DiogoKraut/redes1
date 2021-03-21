CC = gcc
CFLAGS = -Iheader -g -Wall
HEADERS = $(wildcard header/*.h)
OBJECTS = src/cmds.o src/socket.o
TARGET = server client
LIBS = -lm

default: $(TARGET)
all: default clean

%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o src/$@

server: src/server.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $< -o $@

client: src/client.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $< -o $@


clean:
	rm -f src/*.o core

purge:
	rm -f src/*.o
	rm -f $(TARGET)