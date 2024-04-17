CC = gcc

BUILD_DIR=build
BUILD := release

cflags.common := -Wall -Wextra -std=gnu99
cflags.debug := -g -O0
cflags.release := -O2
CFLAGS := ${cflags.${BUILD}} ${cflags.common}

.PHONY: clean all

all: main

main: main.c | $(BUILD_DIR)
	$(CC) $(CLFAGS) $^ -o $(BUILD_DIR)/$@ -lm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm $(BUILD_DIR) -rf
