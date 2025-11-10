/*
 * Copyright (c) 2025 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cpu/o3/comp_simp.hh"

#include "cpu/o3/fu_pool.hh"
#include "debug/CompSimp.hh"

namespace gem5
{

namespace o3
{

// Known ARM multiplication instructions that should be classified as IntMultOp
// Format: "mnemonic" -> "RTL description"
const std::set<std::string> ComputationalSimplification::knownIntMultInstructions = {
    // ARM 32-bit multiplication instructions
    "mul",      // dest = op1 * op2
    "muls",     // dest = op1 * op2; update_flags
    "mla",      // dest = op1 * op2 + op3
    "mlas",     // dest = op1 * op2 + op3; update_flags
    "mls",      // dest = op3 - op1 * op2

    // Signed multiply-accumulate (16-bit operands)
    "smlabb",   // dest = op1.lo * op2.lo + op3
    "smlabt",   // dest = op1.lo * op2.hi + op3
    "smlatb",   // dest = op1.hi * op2.lo + op3
    "smlatt",   // dest = op1.hi * op2.hi + op3
    "smlad",    // dest = op1.hi * op2.hi + op1.lo * op2.lo + op3
    "smladx",   // dest = op1.hi * op2.lo + op1.lo * op2.hi + op3

    // Signed multiply long
    "smlal",    // {dest_hi, dest_lo} = op1 * op2 + {op3_hi, op3_lo}
    "smlals",   // {dest_hi, dest_lo} = op1 * op2 + {op3_hi, op3_lo}; update_flags
    "smlalbb",  // {dest_hi, dest_lo} = op1.lo * op2.lo + {op3_hi, op3_lo}
    "smlalbt",  // {dest_hi, dest_lo} = op1.lo * op2.hi + {op3_hi, op3_lo}
    "smlaltb",  // {dest_hi, dest_lo} = op1.hi * op2.lo + {op3_hi, op3_lo}
    "smlaltt",  // {dest_hi, dest_lo} = op1.hi * op2.hi + {op3_hi, op3_lo}
    "smlald",   // {dest_hi, dest_lo} = op1.hi * op2.hi + op1.lo * op2.lo + {op3_hi, op3_lo}
    "smlaldx",  // {dest_hi, dest_lo} = op1.hi * op2.lo + op1.lo * op2.hi + {op3_hi, op3_lo}

    // Signed multiply-accumulate word
    "smlawb",   // dest = (op1 * op2.lo + (op3 << 16)) >> 16
    "smlawt",   // dest = (op1 * op2.hi + (op3 << 16)) >> 16

    // Signed dual multiply-subtract
    "smlsd",    // dest = op1.lo * op2.lo - op1.hi * op2.hi + op3
    "smlsdx",   // dest = op1.lo * op2.hi - op1.hi * op2.lo + op3
    "smlsld",   // {dest_hi, dest_lo} = op1.lo * op2.lo - op1.hi * op2.hi + {op3_hi, op3_lo}
    "smlsldx",  // {dest_hi, dest_lo} = op1.lo * op2.hi - op1.hi * op2.lo + {op3_hi, op3_lo}

    // Signed most significant word multiply
    "smmla",    // dest = ((op3 << 32) + op1 * op2) >> 32
    "smmlar",   // dest = ((op3 << 32) + op1 * op2 + (1 << 31)) >> 32
    "smmls",    // dest = ((op3 << 32) - op1 * op2) >> 32
    "smmlsr",   // dest = ((op3 << 32) - op1 * op2 + (1 << 31)) >> 32
    "smmul",    // dest = (op1 * op2) >> 32
    "smmulr",   // dest = (op1 * op2 + (1 << 31)) >> 32

    // Signed dual multiply
    "smuad",    // dest = op1.lo * op2.lo + op1.hi * op2.hi
    "smuadx",   // dest = op1.lo * op2.hi + op1.hi * op2.lo

    // Signed multiply (16-bit operands)
    "smulbb",   // dest = op1.lo * op2.lo
    "smulbt",   // dest = op1.lo * op2.hi
    "smultb",   // dest = op1.hi * op2.lo
    "smultt",   // dest = op1.hi * op2.hi

    // Signed multiply long
    "smull",    // {dest_hi, dest_lo} = op1 * op2
    "smulls",   // {dest_hi, dest_lo} = op1 * op2; update_flags

    // Signed multiply word
    "smulwb",   // dest = (op1 * op2.lo) >> 16
    "smulwt",   // dest = (op1 * op2.hi) >> 16

    // Signed dual multiply-subtract
    "smusd",    // dest = op1.lo * op2.lo - op1.hi * op2.hi
    "smusdx",   // dest = op1.lo * op2.hi - op1.hi * op2.lo

    // Unsigned multiply
    "umaal",    // {dest_hi, dest_lo} = op1 * op2 + op3 + op4
    "umlal",    // {dest_hi, dest_lo} = op1 * op2 + {op3_hi, op3_lo}
    "umlals",   // {dest_hi, dest_lo} = op1 * op2 + {op3_hi, op3_lo}; update_flags
    "umull",    // {dest_hi, dest_lo} = op1 * op2
    "umulls",   // {dest_hi, dest_lo} = op1 * op2; update_flags

    // ARM 64-bit multiplication instructions
    "madd",     // dest = op1 + op2 * op3
    "msub",     // dest = op1 - op2 * op3
    "smaddl",   // dest = op1 + sext32(op2) * sext32(op3)
    "smsubl",   // dest = op1 - sext32(op2) * sext32(op3)
    "smulh",    // dest = (op1 * op2) >> 64
    "umaddl",   // dest = op1 + op2 * op3
    "umsubl",   // dest = op1 - op2 * op3
    "umulh"     // dest = (op1 * op2) >> 64
};

// Known ARM ALU instruction mnemonics for validation (add/subtract only)
const std::set<std::string> ComputationalSimplification::knownIntAluInstructions = {
    // Addition instructions
    "add",      // dest = op1 + op2
    "adds",     // dest = op1 + op2; update_flags
    "adc",      // dest = op1 + op2 + carry
    "adcs",     // dest = op1 + op2 + carry; update_flags

    // Subtraction instructions
    "sub",      // dest = op1 - op2
    "subs",     // dest = op1 - op2; update_flags
    "sbc",      // dest = op1 - op2 - !carry
    "sbcs",     // dest = op1 - op2 - !carry; update_flags
    "rsb",      // dest = op2 - op1 (reverse subtract)
    "rsbs",     // dest = op2 - op1; update_flags
    "rsc",      // dest = op2 - op1 - !carry (reverse subtract with carry)
    "rscs",     // dest = op2 - op1 - !carry; update_flags

    // Compare instructions (subtraction without storing result)
    "cmp",      // op1 - op2; update_flags only
    "cmn"       // op1 + op2; update_flags only (compare negative)
};

ComputationalSimplification::ComputationalSimplification(statistics::Group *parent)
    : statistics::Group(parent), stats(this)
{
    // Initialize with reasonable default size - will be resized as needed
    fastPathExecution.resize(32, false);
}

bool
ComputationalSimplification::isInstructionSupported(OpClass capability, const std::string& instName, int numSrcs)
{
    // Computational simplification requires at least 2 source operands for analysis
    if (numSrcs < 2) {
        //DPRINTF(CompSimp, "CompSimp: %s has only %d source operands, need >= 2\n", instName, numSrcs);
        return false;
    }

    if (capability == IntMultOp) {
        if(knownIntMultInstructions.find(instName) != knownIntMultInstructions.end()) {
            return true;
        } else {
            // warn("ComputationalSimplification: Unhandled instruction '%s' - "
            //         "please add to knownIntMultInstructions set", instName);
            // DPRINTF(CompSimp, "UNHANDLED IntMultOp: %s\n", instName);
            return false;
        }
    } else if (capability == IntAluOp) {
        if(knownIntAluInstructions.find(instName) != knownIntAluInstructions.end()) {
            return true;
        } else {
            // warn("ComputationalSimplification: Unhandled instruction '%s' - "
            //         "please add to knownIntAluInstructions set", instName);
            // DPRINTF(CompSimp, "UNHANDLED IntAluOp: %s\n", instName);
            return false;
        }
    }
    return false;
}

ComputationalSimplification::SimplificationResult
ComputationalSimplification::analyzeInstruction(FUPool* fuPool, OpClass capability,
                                               const std::string& instName,
                                               uint64_t op1, uint64_t op2)
{
    SimplificationResult result;
    result.opClass = capability;
    result.op1 = op1;
    result.op2 = op2;
    result.canSimplify = false;

    // Check if this operation class supports computational simplification
    if (!fuPool->supportsCompSimplification(capability)) {
        // No computational simplification available - use normal allocation
        result.fuIndex = fuPool->getUnit(capability);
        result.latency = fuPool->getOpLatency(capability);

        // Update statistics for normal execution
        updateStats(capability, false, Cycles(0));
        return result;
    }

    // Allocate functional unit (always use same FU to model execution pressure)
    result.fuIndex = fuPool->getUnit(capability);

    // If no FU available, return with error
    if (result.fuIndex < 0) {
        result.latency = Cycles(1);
        return result;
    }

    // Check for computational simplification opportunities
    bool canUseSimplification = false;

    if (capability == IntMultOp && isTrivialMultiply(op1, op2)) {
        canUseSimplification = true;
        DPRINTF(CompSimp, "CompSimp: Trivial multiply detected op1=0x%llx, op2=0x%llx\n",
                op1, op2);
    } else if (capability == IntAluOp && isTrivialAlu(op1, op2)) {
        canUseSimplification = true;
        DPRINTF(CompSimp, "CompSimp: Trivial ALU operation detected op1=0x%llx, op2=0x%llx\n",
                op1, op2);
    }

    // Set latency based on simplification analysis
    if (canUseSimplification) {
        result.canSimplify = true;
        result.latency = fuPool->getFastPathLatency(capability);
        setFastPathExecution(result.fuIndex, true);

        // Calculate cycles saved and update statistics
        Cycles normalLat = fuPool->getOpLatency(capability);
        Cycles cyclesSaved = normalLat - result.latency;
        updateStats(capability, true, cyclesSaved);

        DPRINTF(CompSimp, "CompSimp: Using fast-path FU[%d], saved %d cycles (%d->%d)\n",
                result.fuIndex, cyclesSaved, normalLat, result.latency);
    } else {
        result.latency = fuPool->getOpLatency(capability);
        setFastPathExecution(result.fuIndex, false);

        // Update statistics for normal execution
        updateStats(capability, false, Cycles(0));

        DPRINTF(CompSimp, "CompSimp: Using normal execution FU[%d], %d cycles\n",
                result.fuIndex, result.latency);
    }

    return result;
}

bool
ComputationalSimplification::isTrivialMultiply(uint64_t op1, uint64_t op2) const
{
    // ARM multiplication patterns we can optimize:
    // 1. MUL: Reg0 = Reg1 * Reg2 (op1=Reg1, op2=Reg2)
    // 2. MLA: Reg0 = Reg1 * Reg2 + Reg3 (op1=Reg1, op2=Reg2, accumulate separate)
    // 3. MLS: Reg0 = Reg3 - Reg1 * Reg2 (op1=Reg1, op2=Reg2, subtract separate)

    // Fast-path cases for multiplication operands:
    // - Any operand is 0: result is 0 (or just accumulator for MLA/MLS)
    // - Any operand is 1: result is other operand
    // - Any operand is power of 2: can use shift instead
    return (op1 == 0 || op2 == 0 ||                              // multiply by zero
            op1 == 1 || op2 == 1 ||                              // multiply by one
            (op1 != 0 && (op1 & (op1 - 1)) == 0) ||             // op1 is power of 2
            (op2 != 0 && (op2 & (op2 - 1)) == 0));              // op2 is power of 2
}

bool
ComputationalSimplification::isTrivialAlu(uint64_t op1, uint64_t op2) const
{
    // ARM ALU operations we can optimize:
    // ADD: result = op1 + op2  -> if op2 == 0, result = op1 (identity)
    // SUB: result = op1 - op2  -> if op2 == 0, result = op1 (identity)
    // RSB: result = op2 - op1  -> if op1 == 0, result = op2 (identity)

    // Fast-path cases for ALU operations:
    return (op1 == 0 || op2 == 0);  // Adding/subtracting 0 is identity operation
}

void
ComputationalSimplification::setFastPathExecution(int fu_idx, bool is_fast_path)
{
    // Ensure tracking vector is large enough
    if (fu_idx >= 0 && static_cast<size_t>(fu_idx) >= fastPathExecution.size()) {
        resizeTracking(fu_idx + 1);
    }

    if (fu_idx >= 0) {
        fastPathExecution[fu_idx] = is_fast_path;
    }
}

bool
ComputationalSimplification::isFastPathExecution(int fu_idx) const
{
    if (fu_idx < 0 || static_cast<size_t>(fu_idx) >= fastPathExecution.size()) {
        return false;
    }
    return fastPathExecution[fu_idx];
}

void
ComputationalSimplification::clearFastPathExecution(int fu_idx)
{
    if (fu_idx >= 0 && static_cast<size_t>(fu_idx) < fastPathExecution.size()) {
        fastPathExecution[fu_idx] = false;
    }
}

void
ComputationalSimplification::resizeTracking(size_t num_fus)
{
    fastPathExecution.resize(num_fus, false);
}

void
ComputationalSimplification::updateStats(OpClass capability, bool used_fast_path,
                                        Cycles cycles_saved)
{
    if (capability == IntMultOp) {
        if (used_fast_path) {
            stats.fastPathMultiplications++;
        } else {
            stats.normalMultiplications++;
        }
    } else if (capability == IntAluOp) {
        if (used_fast_path) {
            stats.fastPathAlu++;
        } else {
            stats.normalAlu++;
        }
    }

    if (used_fast_path) {
        stats.cyclesSavedCompSimp += cycles_saved;
    }
}

ComputationalSimplification::CompSimpStats::CompSimpStats(statistics::Group *parent)
    : statistics::Group(parent),
    // Statistics for computational simplification performance tracking
    ADD_STAT(fastPathMultiplications, statistics::units::Count::get(),
             "Number of multiplication instructions using 1-cycle fast-path execution"),
    ADD_STAT(normalMultiplications, statistics::units::Count::get(),
             "Number of multiplication instructions using normal 3-cycle execution"),
    ADD_STAT(fastPathAlu, statistics::units::Count::get(),
             "Number of ALU instructions using fast-path optimized execution"),
    ADD_STAT(normalAlu, statistics::units::Count::get(),
             "Number of ALU instructions using normal execution"),
    ADD_STAT(cyclesSavedCompSimp, statistics::units::Count::get(),
             "Total cycles saved due to computational simplification optimizations")
{
}

} // namespace o3
} // namespace gem5