GEM5_PATH = /home/rgangar/Documents/gem5_claude/
ISA = arm64
GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O1 -march=armv8.4-a -Wall -I$(GEM5_PATH)/include
LDFLAGS = -L$(GEM5_PATH)/util/m5/build/$(ISA)/out

all: hello-static hello-dynamic hello-static.S hello-dynamic.S silent_store_test-static silent_store_test-dynamic silent_store_test-static.S silent_store_test-dynamic.S memory_intensive_silent-static linked_list_silent-static matrix_silent-static sorting_silent-static

hello-static: hello.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

hello-dynamic: hello.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

hello-static.S: hello-static
	$(OBJDUMP) -d $< > $@

hello-dynamic.S: hello-dynamic
	$(OBJDUMP) -d $< > $@

silent_store_test-static: silent_store_test.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

silent_store_test-dynamic: silent_store_test.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

silent_store_test-static.S: silent_store_test-static
	$(OBJDUMP) -d $< > $@

silent_store_test-dynamic.S: silent_store_test-dynamic
	$(OBJDUMP) -d $< > $@

memory_intensive_silent-static: memory_intensive_silent.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

linked_list_silent-static: linked_list_silent.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

matrix_silent-static: matrix_silent.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

sorting_silent-static: sorting_silent.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS) -static

clean:
	rm -f hello-static hello-dynamic hello-static.S hello-dynamic.S silent_store_test-static silent_store_test-dynamic memory_intensive_silent-static linked_list_silent-static matrix_silent-static sorting_silent-static
