CC=gcc
CFLAGS=-Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS=-shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: all clean

all: libnand.so tests

nand_helper.o: nand_helper.c nand_helper.h
	$(CC) $(CFLAGS) -c nand_helper.c

nand_example.o: nand_example.c nand.h memory_tests.h
	$(CC) $(CFLAGS) -c nand_example.c

memory_tests.o: memory_tests.c memory_tests.h
	$(CC) $(CFLAGS) -c memory_tests.c

nand.o: nand.c nand.h
	$(CC) $(CFLAGS) -c nand.c

libnand.so: nand.o memory_tests.o nand_helper.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o libnand.so $^

#Builds basic tests for libnand.so library
tests: nand_example.o libnand.so
	$(CC) $(CFLAGS) -o tests -g nand_example.o -L$(CURDIR) -Wl,-rpath=$(CURDIR) -lnand

#Cleans elements created during building and linking process
clean:
	rm -f tests libnand.so *.o
