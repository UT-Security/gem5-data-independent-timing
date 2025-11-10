GEM5_PATH = /home/rgangar/Documents/gem5
ISA = arm64
GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O2 -march=armv8-a -Wall
LDFLAGS = -static

all: lvp_benchmark lvp_benchmark.S

lvp_benchmark: lvp_benchmark.c
	$(GXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

lvp_benchmark.S: lvp_benchmark
	$(OBJDUMP) -d $< > $@

clean:
	rm -f lvp_benchmark lvp_benchmark.S

.PHONY: all clean
