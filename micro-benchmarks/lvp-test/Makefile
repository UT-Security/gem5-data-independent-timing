GEM5_PATH = /home/rgangar/Documents/gem5
ISA = arm64
GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O2 -march=armv8.4-a -Wall
LDFLAGS = -static
GEM5_ROOT = /home/rgangar/Documents/gem5
M5_LIB = $(GEM5_ROOT)/util/m5/build/arm64/out/libm5.a
M5_INCLUDE = -I$(GEM5_ROOT)/include

all: lvp_benchmark lvp_benchmark.S

lvp_benchmark: lvp_benchmark.c
	$(GXX) $(CFLAGS) $(M5_INCLUDE) -o $@ $< $(M5_LIB) $(LDFLAGS)

lvp_benchmark.S: lvp_benchmark
	$(OBJDUMP) -d $< > $@

clean:
	rm -f lvp_benchmark lvp_benchmark.S

.PHONY: all clean
