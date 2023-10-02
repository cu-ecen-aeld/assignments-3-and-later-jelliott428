ifdef CROSS_COMPILE
	CC=$(CROSS_COMPILE)gcc
else
	CC=gcc
endif

#CFLAGS= -g -Wall -c
OBJECTS = writer.o

writer: writer.o
	$(CC) -o writer $(OBJECTS)

.PHONY: all
all: writer.o
	$(CC) -o writer $(OBJECTS)


.PHONY: clean
clean:
	rm -f $(wildcard *.o) writer
