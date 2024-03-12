CC = gcc

BUILD_DIR=build
BUILD := debug

cflags.common := -Wall -Wextra -std=c17
cflags.debug := -g -O0
cflags.release := -O2
CFLAGS := ${cflags.${BUILD}} ${cflags.common}

.PHONY: clean all collatz_static collatz_shared main_static main_shared main_dynamic

all: main_static main_shared main_dynamic

main_static: collatz_static main.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ main.c $(BUILD_DIR)/libcollatz_static.a

main_shared: collatz_shared main.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ main.c -L./build/ -lcollatz_shared -Wl,-rpath=./build

main_dynamic: collatz_shared main.c
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ main.c -ldl  -D USE_LIB_DYNAMIC -Wl,-rpath=./build


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


collatz_static:  $(BUILD_DIR) $(BUILD_DIR)/libcollatz_static.a
collatz_shared: $(BUILD_DIR) $(BUILD_DIR)/libcollatz_shared.so

# static library
# first we need compiled objects files(prerequisite)
# then we create an archive using ar
#  	r -> insert the files into archive with replacement
#  	c -> create archive if does not exist
#  	s -> add index to archive or update if it
# which then can be statically linked
$(BUILD_DIR)/libcollatz_static.a: collatz/collatz.c
	$(CC) $(CFLAGS) -c $^ -o $(BUILD_DIR)/collatz.o
	ar rcs $@ $(BUILD_DIR)/collatz.o

# shared library
# when creating a object files we must pass -fPIC
# PIC - Position Independent Code,
# the code is generated in a way that it can be loaded and executed at any
# memory address without the need for runtime relocation. This is achieved
# through indirect addressing of data and functions.
$(BUILD_DIR)/libcollatz_shared.so: collatz/collatz.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $(BUILD_DIR)/collatz.o
	$(CC) -shared -o $@ $(BUILD_DIR)/collatz.o


clean:
	rm $(BUILD_DIR) -rf


# objdump -t -> symbol table
# objdump -d -> disassembly
