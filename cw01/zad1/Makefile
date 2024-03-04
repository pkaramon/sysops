CC = gcc
CFLAGS = -Wall -std=c17 -g

.PHONY: clean all

all: countdown

SOURCES = countdown.c 
OBJECTS = $(SOURCES:.c=.o)

countdown: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS) countdown
