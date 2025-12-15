# DIT Toggle Performance Benchmark

This microbenchmark measures the performance overhead of toggling the FEAT_DIT (Data Independent Timing) PSTATE bit.

## Purpose

FEAT_DIT is typically used in security-critical code sections. This benchmark helps understand:
1. The serialization cost of MSR DIT instructions
2. Pipeline stalls caused by toggling DIT mode
3. Performance difference between immediate vs register forms
4. Overhead compared to baseline (no DIT toggling)

## Benchmark Structure

The benchmark consists of three tests:

### Test 1: Baseline (No DIT Toggling)
- Simple arithmetic operations in a loop
- No MSR DIT instructions
- Establishes baseline performance

### Test 2: DIT Toggle with Immediate Form
- Same arithmetic operations
- Toggles DIT using `MSR DIT, #1` and `MSR DIT, #0`
- Measures overhead of immediate form

### Test 3: DIT Toggle with Register Form
- Same arithmetic operations
- Toggles DIT using `MSR DIT, Xn`
- Measures overhead of register form

Each test runs 100,000 iterations with:
- 2 MSR DIT instructions per iteration (on/off)
- 4 arithmetic operations per iteration

## Building

```bash
make
```

This will:
- Compile `dit_toggle_bench.c` for AArch64
- Generate disassembly in `dit_toggle_bench.S`

## Running

```bash
chmod +x run_bench.sh
./run_bench.sh
```

Or manually:
```bash
$GEM5_ROOT/build/ARM/gem5.opt --outdir=output \
    $GEM5_ROOT/custom-configs/se_custom_arm_binary.py \
    --input-bin dit_toggle_bench
```

## Interpreting Results

### Key Stats to Compare

From `output/stats.txt`:

```bash
# Total cycles for each test
grep "simTicks" output/stats.txt

# Instructions per cycle
grep "system.cpu.ipc" output/stats.txt

# Committed instructions
grep "system.cpu.commit.committedInsts" output/stats.txt
```

### Expected Observations

**MSR DIT Serialization:**
- MSR DIT has `IsSerializeAfter` flag → pipeline barrier
- All instructions after MSR must wait for completion
- Expect significant cycle increase vs baseline

**Performance Impact:**
```
Overhead = (DIT_toggle_cycles - Baseline_cycles) / Baseline_cycles * 100%
```

**Immediate vs Register Form:**
- Immediate form: `MSR DIT, #1` encodes value directly
- Register form: `MSR DIT, Xn` requires register read
- Both are serializing, but register form may have extra latency

### Example Analysis

If baseline takes 400K cycles and DIT toggle takes 600K cycles:
- Overhead: 50%
- Per MSR cost: (200K extra cycles) / (200K MSR instructions) = 1 cycle per MSR
- This indicates efficient serialization handling

## Key Implementation Details

### MSR DIT Serialization Flags

From gem5 source:
- **MSR DIT, #imm**: `["IsSerializeAfter", "IsNonSpeculative"]`
- **MSR DIT, Xn**: `["IsSerializeAfter", "IsNonSpeculative"]`
- **MRS Xn, DIT**: `["IsSerializeBefore"]`

### Register Form Encoding

For `MSR DIT, Xn`:
- Value must be in bit 24 position (CPSR.DIT bit)
- Example: `Xn = (1UL << 24)` to set DIT

For `MSR DIT, #imm`:
- Immediate value (0 or 1) is shifted by gem5 to bit 24

## Files

- `dit_toggle_bench.c` - Benchmark source code
- `Makefile` - Build configuration
- `run_bench.sh` - gem5 execution script
- `README.md` - This file
- `output/` - gem5 simulation results (created after running)

## Notes

- The benchmark uses `-O2` optimization to keep code realistic
- Static linking ensures standalone execution in gem5 SE mode
- ARMv8.4-a is required for FEAT_DIT support