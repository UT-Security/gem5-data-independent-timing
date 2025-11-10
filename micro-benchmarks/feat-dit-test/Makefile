GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O0 -march=armv8.4-a -Wall -g
LDFLAGS = -static

all: test_dit test_dit.S

test_dit: test_dit.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_dit.S: test_dit
	$(OBJDUMP) -d $< > $@

clean:
	rm -f test_dit test_dit.S

.PHONY: all clean
