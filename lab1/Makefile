CXX = gcc
CFLAGS = -Wall -Wextra -g

all: countdown

SOURCES = countdown.c 
OBJECTS = $(SOURCES:.c=.o)

countdown: $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm $(OBJECTS) countdown

.PHONY: clean