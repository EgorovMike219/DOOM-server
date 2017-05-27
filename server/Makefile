CC=gcc
CFLAGS=-c -Wall -lpthread -pthread -std=c99
LDFLAGS=-lpthread -pthread
SOURCES=main.c dconnect.c game.c level.c unit.c weapon.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
