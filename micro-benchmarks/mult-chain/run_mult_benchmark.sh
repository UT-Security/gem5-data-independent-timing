#!/bin/bash

# Script to run multiplication chain benchmark with gem5
# Usage: ./run_mult_benchmark.sh [baseline|compsimp]

GEM5_ROOT="/home/rgangar/Documents/gem5"
CONFIG_SCRIPT="$GEM5_ROOT/custom-configs/se_custom_arm_binary.py"
BENCHMARK="$GEM5_ROOT/micro-benchmarks/mult-chain/mult_benchmark"
GEM5_BIN="$GEM5_ROOT/build/ARM/gem5.opt"

MODE=${1:-baseline}

if [ ! -f "$BENCHMARK" ]; then
    echo "Error: Benchmark binary not found at $BENCHMARK"
    echo "Run 'make' in the mult-chain directory first"
    exit 1
fi

if [ ! -f "$GEM5_BIN" ]; then
    echo "Error: gem5 binary not found at $GEM5_BIN"
    echo "Build gem5 first with: scons build/ARM/gem5.opt -j\$(nproc)"
    exit 1
fi

# Set output directory based on mode
OUTPUT_DIR="$GEM5_ROOT/micro-benchmarks/mult-chain/$MODE"

echo "========================================="
echo "Running Multiplication Chain Benchmark"
echo "Mode: $MODE"
echo "========================================="
echo "Output directory: $OUTPUT_DIR"
echo ""

if [ "$MODE" == "baseline" ]; then
    echo "NOTE: Ensure computational simplification is DISABLED"
    echo "      in src/cpu/o3/inst_queue.cc:827"
    echo "      (Change: if (fuPool->getCompSimp()... to if (false && fuPool->getCompSimp()...)"
elif [ "$MODE" == "compsimp" ]; then
    echo "NOTE: Ensure computational simplification is ENABLED"
    echo "      in src/cpu/o3/inst_queue.cc:827"
    echo "      (Change: if (false && fuPool->getCompSimp()... to if (fuPool->getCompSimp()...)"
else
    echo "Error: Invalid mode '$MODE'. Use 'baseline' or 'compsimp'"
    exit 1
fi

echo ""
read -p "Press Enter to continue or Ctrl+C to abort..."

# Run gem5
eval "$GEM5_BIN --outdir=$OUTPUT_DIR $CONFIG_SCRIPT --input-bin '$BENCHMARK'"

echo ""
echo "========================================="
echo "Simulation Complete"
echo "========================================="
echo "Results in: $OUTPUT_DIR/stats.txt"
echo ""
echo "Key stats to check:"
echo "  - board.processor.cores.core.ipc"
echo "  - board.processor.cores.core.numCycles"
echo "  - board.processor.cores.core.fuPool.compSimp.fastPathMultiplications"
echo "  - board.processor.cores.core.fuPool.compSimp.normalMultiplications"
echo "  - board.processor.cores.core.fuPool.compSimp.cyclesSavedCompSimp"
echo ""
