#!/bin/bash

# Script to run DIT toggle performance benchmark with gem5

GEM5_ROOT="/home/rgangar/Documents/gem5"
CONFIG_SCRIPT="$GEM5_ROOT/custom-configs/se_custom_arm_binary.py"
TEST_BINARY="$GEM5_ROOT/micro-benchmarks/dit-toggle-perf/dit_toggle_bench"
GEM5_BIN="$GEM5_ROOT/build/ARM/gem5.opt"
OUTPUT_DIR="$GEM5_ROOT/micro-benchmarks/dit-toggle-perf/output"

echo "========================================="
echo "DIT Toggle Performance Benchmark"
echo "========================================="
echo ""

# Check if binary exists
if [ ! -f "$TEST_BINARY" ]; then
    echo "Error: Benchmark binary not found at $TEST_BINARY"
    echo "Run 'make' in the dit-toggle-perf directory first"
    exit 1
fi

# Check if gem5 binary exists
if [ ! -f "$GEM5_BIN" ]; then
    echo "Error: gem5 binary not found at $GEM5_BIN"
    echo "Build gem5 first with: scons build/ARM/gem5.opt -j$(nproc)"
    exit 1
fi

echo "Benchmark binary: $TEST_BINARY"
echo "Output directory: $OUTPUT_DIR"
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Run gem5
echo "Running gem5..."
$GEM5_BIN --outdir="$OUTPUT_DIR" "$CONFIG_SCRIPT" --input-bin "$TEST_BINARY"

echo ""
echo "========================================="
echo "Simulation Complete"
echo "========================================="
echo ""
echo "Output saved to: $OUTPUT_DIR"
echo ""
echo "To view results:"
echo "  Terminal output: cat $OUTPUT_DIR/simout.txt"
echo "  Performance stats: grep -E 'numCycles|ipc' $OUTPUT_DIR/stats.txt"
echo ""
echo "Key metrics to compare:"
echo "  - simTicks: Total simulation time"
echo "  - numCycles: Total CPU cycles"
echo "  - system.cpu.ipc: Instructions per cycle"
echo ""
