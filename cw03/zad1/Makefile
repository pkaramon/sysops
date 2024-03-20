CC = gcc

BUILD_DIR=build
BUILD := release

cflags.common := -Wall -Wextra -std=c17
cflags.debug := -g -O0
cflags.release := -O2
CFLAGS := ${cflags.${BUILD}} ${cflags.common}
TEST_DIR := ./test_files
TEST_RESULTS := tests-results.txt

.PHONY: clean all test

all: reverse_bytes reverse_blocks

reverse_bytes: reverse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@

reverse_blocks: reverse.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $(BUILD_DIR)/$@ -DBLOCK_SIZE=1024

$(BUILD_DIR):
	mkdir -p ./build

test_perf_bytes: reverse_bytes
	@./tester ./$(BUILD_DIR)/reverse_bytes performance | tee -a $(TEST_RESULTS)

test_func_bytes: reverse_bytes
	@./tester ./$(BUILD_DIR)/reverse_bytes functional | tee -a $(TEST_RESULTS)

test_perf_blocks: reverse_blocks
	@./tester ./$(BUILD_DIR)/reverse_blocks performance | tee -a $(TEST_RESULTS)

test_func_blocks: reverse_blocks
	@./tester ./$(BUILD_DIR)/reverse_blocks functional | tee -a $(TEST_RESULTS)

clean:
	rm $(BUILD_DIR) -rf
	rm $(TEST_DIR) -rf

