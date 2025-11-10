# FEAT_DIT Test

This test verifies that ARM FEAT_DIT (Data Independent Timing) is correctly implemented in gem5.

## What is FEAT_DIT?

FEAT_DIT is an ARMv8.4-A security feature that ensures data-independent timing for certain instructions to mitigate timing side-channel attacks. It's controlled by the PSTATE.DIT bit (bit 24).

## Test Coverage

The test program verifies:

1. **Register Access**: MSR/MRS DIT instructions work correctly
2. **Bit Manipulation**: DIT bit can be set and cleared
3. **Immediate Form**: MSR DIT, #<imm> instructions work
4. **CPSR Integration**: DIT is properly part of CPSR

**Note**: The test skips reading ID_AA64PFR0_EL1 because it requires EL1 privileges and will fault in user mode (SE mode). The presence of FEAT_DIT is verified by the fact that MSR/MRS DIT instructions work without faulting.

## Building and Running

### 1. Build the test:
```bash
cd /home/rgangar/Documents/gem5/micro-benchmarks/feat-dit-test
make
```

### 2. Build gem5 (if not already built):
```bash
cd /home/rgangar/Documents/gem5
scons build/ARM/gem5.opt -j$(nproc)
```

### 3. Run the test:
```bash
./run_test.sh
```

### 4. Check results:
```bash
cat output/simout.txt | grep -A 100 "FEAT_DIT Test"
```

## Expected Output

If FEAT_DIT is correctly implemented, you should see:

```
=== FEAT_DIT Test Program ===

Testing FEAT_DIT (Data Independent Timing) implementation
Note: Running in user mode (EL0), testing DIT register access

Test 1: Reading initial DIT value...
  Initial DIT = 0x0 (bit 0 = 0)

Test 2: Setting DIT to 1...
  DIT after MSR DIT, #1 = 0x1 (bit 0 = 1)
  ✓ DIT successfully set to 1

Test 3: Clearing DIT to 0...
  DIT after MSR DIT, #0 = 0x0 (bit 0 = 0)
  ✓ DIT successfully cleared to 0

Test 4: Testing MSR DIT, #1 (immediate form)...
  DIT after MSR DIT, #1 = 0x1 (bit 0 = 1)
  ✓ MSR DIT, #1 (immediate) works

Test 5: Testing MSR DIT, #0 (immediate form)...
  DIT after MSR DIT, #0 = 0x0 (bit 0 = 0)
  ✓ MSR DIT, #0 (immediate) works

Test 6: Verifying DIT is part of CPSR...
  NZCV should be unchanged when DIT changes
  NZCV before: 0x..., after: 0x...
  ✓ DIT changes don't affect NZCV (as expected)

=== All FEAT_DIT Tests Passed! ===
```

## Notes

- This is a **functional test** - it verifies that the DIT feature is architecturally visible
- gem5 implements DIT as a NOP for timing (doesn't affect execution timing) since gem5 doesn't model data-dependent timing variations
- The test uses `-march=armv8.4-a` to enable DIT instructions in the assembler
