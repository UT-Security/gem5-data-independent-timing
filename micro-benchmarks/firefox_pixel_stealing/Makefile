GXX = aarch64-linux-gnu-gcc
OBJDUMP = aarch64-linux-gnu-objdump
CFLAGS = -O2 -march=armv8.4-a -Wall -g
LDFLAGS = -static

# gem5 m5ops library
GEM5_ROOT = /home/rgangar/Documents/gem5
M5_LIB = $(GEM5_ROOT)/util/m5/build/arm64/out/libm5.a
M5_INCLUDE = -I$(GEM5_ROOT)/include

all: convolvePixel convolvePixel.S simple_madd simple_madd.S

convolvePixel: convolvePixel.c
	$(GXX) $(CFLAGS) $(M5_INCLUDE) -o $@ $< $(M5_LIB) $(LDFLAGS)

convolvePixel.S: convolvePixel
	$(OBJDUMP) -d $< > $@

simple_madd: simple_madd.c
	$(GXX) $(CFLAGS) $(M5_INCLUDE) -o $@ $< $(M5_LIB) $(LDFLAGS)

simple_madd.S: simple_madd
	$(OBJDUMP) -d $< > $@

clean:
	rm -f convolvePixel convolvePixel.S simple_madd simple_madd.S

.PHONY: all clean
