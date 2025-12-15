CC = aarch64-linux-gnu-gcc
GEM5_PATH = /home/rgangar/Documents/gem5
TARGET_ISA = arm64

CFLAGS = -O2 -march=armv8.4-a -Wall -g -static
CFLAGS += -I$(GEM5_PATH)/include
LDFLAGS += -L$(GEM5_PATH)/util/m5/build/$(TARGET_ISA)/out -lm5

OBJDUMP = aarch64-linux-gnu-objdump

TARGET = dit_toggle_bench
SRC = dit_toggle_bench.c
ASM = $(TARGET).S

all: $(TARGET) $(ASM)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(ASM): $(TARGET)
	$(OBJDUMP) -d $< > $@

clean:
	rm -f $(TARGET) $(ASM)

.PHONY: all clean
