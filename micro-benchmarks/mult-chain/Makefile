GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O1 -march=armv8-a -Wall -fno-tree-loop-optimize
LDFLAGS = -static

all: mult_benchmark mult_benchmark.S

mult_benchmark: mult_benchmark.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

mult_benchmark.S: mult_benchmark
	$(OBJDUMP) -d $< > $@

clean:
	rm -f mult_benchmark mult_benchmark.S

.PHONY: all clean
