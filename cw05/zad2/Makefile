CC = gcc

BUILD_DIR=build
BUILD := release

cflags.common := -Wall -Wextra -std=gnu99
cflags.debug := -g -O0
cflags.release := -O2
CFLAGS := ${cflags.${BUILD}} ${cflags.common}

.PHONY: clean all

all: sender catcher

catcher: catcher.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@

sender: sender.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm $(BUILD_DIR) -rf
