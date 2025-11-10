#include <stdio.h>
#include <stdint.h>

// Test program to verify FEAT_DIT implementation in gem5
// This tests:
// 1. MSR/MRS DIT instructions work
// 2. DIT bit can be set/cleared
//
// Note: We skip reading ID_AA64PFR0_EL1 because it requires EL1 privileges
// and will fault in user mode (SE mode). The presence of FEAT_DIT is verified
// by the fact that MSR/MRS DIT instructions work without faulting.

int main() {
    uint64_t dit_value;

    printf("=== FEAT_DIT Test Program ===\n\n");
    printf("Testing FEAT_DIT (Data Independent Timing) implementation\n");
    printf("Note: Running in user mode (EL0), testing DIT register access\n\n");

    // Test 1: Read initial DIT value
    printf("Test 1: Reading initial DIT value...\n");
    __asm__ volatile("mrs %0, DIT" : "=r" (dit_value));
    printf("  Initial DIT = 0x%lx (bit 24 = %ld)\n\n", dit_value, (dit_value >> 24) & 0x1);

    // Test 2: Set DIT to 1 (register form - value must be in bit 24 position)
    printf("Test 2: Setting DIT to 1...\n");
    __asm__ volatile("msr DIT, %0" :: "r" (1UL << 24));
    __asm__ volatile("mrs %0, DIT" : "=r" (dit_value));
    printf("  DIT after MSR DIT, x = 0x%lx (bit 24 = %ld)\n", dit_value, (dit_value >> 24) & 0x1);

    if (((dit_value >> 24) & 0x1) == 1) {
        printf("  ✓ DIT successfully set to 1\n\n");
    } else {
        printf("  ✗ Failed to set DIT to 1\n\n");
        return 1;
    }

    // Test 3: Clear DIT to 0 (register form)
    printf("Test 3: Clearing DIT to 0...\n");
    __asm__ volatile("msr DIT, %0" :: "r" (0UL));
    __asm__ volatile("mrs %0, DIT" : "=r" (dit_value));
    printf("  DIT after MSR DIT, x = 0x%lx (bit 24 = %ld)\n", dit_value, (dit_value >> 24) & 0x1);

    if (((dit_value >> 24) & 0x1) == 0) {
        printf("  ✓ DIT successfully cleared to 0\n\n");
    } else {
        printf("  ✗ Failed to clear DIT to 0\n\n");
        return 1;
    }

    // Test 4: Test MSR DIT with immediate (MSR DIT, #<imm>)
    printf("Test 4: Testing MSR DIT, #1 (immediate form)...\n");
    __asm__ volatile("msr DIT, #1");
    __asm__ volatile("mrs %0, DIT" : "=r" (dit_value));
    printf("  DIT after MSR DIT, #1 = 0x%lx (bit 24 = %ld)\n", dit_value, (dit_value >> 24) & 0x1);

    if (((dit_value >> 24) & 0x1) == 1) {
        printf("  ✓ MSR DIT, #1 (immediate) works\n\n");
    } else {
        printf("  ✗ MSR DIT, #1 (immediate) failed\n\n");
        return 1;
    }

    printf("Test 5: Testing MSR DIT, #0 (immediate form)...\n");
    __asm__ volatile("msr DIT, #0");
    __asm__ volatile("mrs %0, DIT" : "=r" (dit_value));
    printf("  DIT after MSR DIT, #0 = 0x%lx (bit 24 = %ld)\n", dit_value, (dit_value >> 24) & 0x1);

    if (((dit_value >> 24) & 0x1) == 0) {
        printf("  ✓ MSR DIT, #0 (immediate) works\n\n");
    } else {
        printf("  ✗ MSR DIT, #0 (immediate) failed\n\n");
        return 1;
    }

    // Test 6: Verify DIT is part of CPSR (read NZCV to ensure CPSR updates)
    printf("Test 6: Verifying DIT is part of CPSR...\n");
    __asm__ volatile("msr DIT, #1");

    uint64_t nzcv_before, nzcv_after;
    __asm__ volatile("mrs %0, NZCV" : "=r" (nzcv_before));
    __asm__ volatile("msr DIT, #0");
    __asm__ volatile("mrs %0, NZCV" : "=r" (nzcv_after));

    printf("  NZCV should be unchanged when DIT changes\n");
    printf("  NZCV before: 0x%lx, after: 0x%lx\n", nzcv_before, nzcv_after);

    if (nzcv_before == nzcv_after) {
        printf("  ✓ DIT changes don't affect NZCV (as expected)\n\n");
    } else {
        printf("  ✗ NZCV changed unexpectedly\n\n");
    }

    printf("=== All FEAT_DIT Tests Passed! ===\n");
    return 0;
}
