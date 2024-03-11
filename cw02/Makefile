CC = gcc
CFLAGS = -Wall -std=c17 -g -O0

.PHONY: clean all

all: main_dynamic

SOURCES = main.c collatz.c
OBJECTS = $(SOURCES:.c=.o)

main_static: main.o libcollatz.a
	$(CC) $(CFLAGS) $^ -o $@

main_shared: main.o libcollatz.so.1 libcollatz.so
	$(CC) $(CFLAGS) main.o -lcollatz -L./ -Wl,-rpath,. -o $@

main_dynamic: main.c libcollatz.so.1 libcollatz.so
	$(CC) $(CFLAGS) main.c -lcollatz -L./ -D USE_LIB_DYNAMIC -Wl,-rpath,. -o $@


libcollatz.a: collatz.c
	$(CC) $(CFLAGS) -c collatz.c -o collatz.o
	ar rcs $@ collatz.o

libcollatz.so.1.0.0: collatz.c
	$(CC) $(CFLAGS) -fPIC -c collatz.c -o collatz.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libcollatz.so.1 -o $@ collatz.o -lc

libcollatz.so: libcollatz.so.1.0.0
	ln -s $^ $@

libcollatz.so.1: libcollatz.so.1.0.0
	ln -s $^ $@

clean:
	rm *.a $(OBJECTS) main_static main_shared main_dynamic libcollatz.*

# objdump -t
