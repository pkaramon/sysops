CC = gcc

BUILD_DIR=build

BUILD := debug

cflags.common := -Wall -Wextra -std=c17
cflags.debug := -g -O0
cflags.release := -O2
CFLAGS := ${cflags.${BUILD}} ${cflags.common}

.PHONY: clean all

all: client server

client: client.c socket_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@  -lrt  -lpthread

server: server.c socket_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@  -lrt  -lpthread


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm $(BUILD_DIR) -rf
