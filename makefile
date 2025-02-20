CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: all clean

all: libnand.so

libnand.so: nand.o linked_list.o memory_tests.o
	gcc $(LD_FLAGS) -shared -o libnand.so memory_tests.o nand.o linked_list.o
linked_list.o: linked_list.c nand.h linked_list.h
	gcc $(CFLAGS) -c linked_list.c
memory_tests.o: memory_tests.c
	gcc $(CFLAGS) -c memory_tests.c
nand.o: nand.c nand.h linked_list.h
	gcc $(CFLAGS) -c nand.c
clean:
	rm -f *.o libnand.so