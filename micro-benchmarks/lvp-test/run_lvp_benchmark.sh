#!/bin/bash

# Script to run LVP benchmark with gem5
# Usage: ./run_lvp_benchmark.sh [baseline|lvp]

GEM5_ROOT="/home/rgangar/Documents/gem5"
CONFIG_SCRIPT="$GEM5_ROOT/micro-benchmarks/se_custom_arm_binary.py"
BENCHMARK="$GEM5_ROOT/micro-benchmarks/lvp-test/lvp_benchmark"
GEM5_BIN="$GEM5_ROOT/build/ARM/gem5.opt"

MODE=${1:-baseline}

if [ ! -f "$BENCHMARK" ]; then
    echo "Error: Benchmark binary not found at $BENCHMARK"
    echo "Run 'make' in the lvp-test directory first"
    exit 1
fi

if [ ! -f "$GEM5_BIN" ]; then
    echo "Error: gem5 binary not found at $GEM5_BIN"
    echo "Build gem5 first with: scons build/ARM/gem5.opt -j\$(nproc)"
    exit 1
fi

# Set output directory based on mode
OUTPUT_DIR="$GEM5_ROOT/micro-benchmarks/lvp-test/{MODE}"

echo "========================================="
echo "Running LVP Benchmark - Mode: $MODE"
echo "========================================="
echo "Output directory: $OUTPUT_DIR"
echo ""

if [ "$MODE" == "baseline" ]; then
    echo "NOTE: Ensure LVP is DISABLED in custom-configs/components/cpus/O3_ARM_v8.py"
    echo "      (lines 194-210 should be commented out)"
elif [ "$MODE" == "lvp" ]; then
    echo "NOTE: Ensure LVP is ENABLED in custom-configs/components/cpus/O3_ARM_v8.py"
    echo "      (lines 194-210 should be uncommented)"
else
    echo "Error: Invalid mode '$MODE'. Use 'baseline' or 'lvp'"
    exit 1
fi

echo ""
read -p "Press Enter to continue or Ctrl+C to abort..."

# Run gem5
eval "../../build/ARM/gem5.opt --outdir=$MODE --debug-flags=DIT ../../custom-configs/se_custom_arm_binary.py --input-bin 'lvp_benchmark'"

echo ""
echo "========================================="
echo "Simulation Complete"
echo "========================================="
echo "Results in: $OUTPUT_DIR/stats.txt"
echo ""
echo "Key stats to check:"
echo "  - system.cpu.ipc                    (should be HIGHER with LVP)"
echo "  - system.cpu.numCycles              (should be LOWER with LVP)"
echo "  - system.cpu.lvp.numConstLoads      (LVP mode only)"
echo "  - system.cpu.lvp.predictionAccuracy (LVP mode only)"
echo "  - system.cpu.lvp.numSquashes        (LVP mode only)"
