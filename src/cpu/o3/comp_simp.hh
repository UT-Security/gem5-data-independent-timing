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

#ifndef __CPU_O3_COMP_SIMP_HH__
#define __CPU_O3_COMP_SIMP_HH__

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "base/statistics.hh"
#include "cpu/op_class.hh"
#include "sim/sim_object.hh"

namespace gem5
{

namespace o3
{

class FUPool;

/**
 * Computational Simplification Optimization Module
 *
 * This class implements fast-path execution for instructions with trivial operands.
 * It detects patterns like:
 * - Multiplication by 0, 1, or powers of 2 (3 cycles -> 1 cycle)
 * - Addition/subtraction by 0 (maintains 1 cycle but tracks optimization)
 *
 * The optimization maintains functional unit pressure modeling by always
 * allocating the same FU, but reduces execution latency for trivial cases.
 */
class ComputationalSimplification : public statistics::Group
{
  public:
    /**
     * Result of computational simplification analysis
     */
    struct SimplificationResult
    {
        bool canSimplify;       /** Can this instruction use fast-path? */
        int fuIndex;            /** Allocated functional unit index */
        Cycles latency;         /** Execution latency (normal or fast-path) */
        OpClass opClass;        /** Operation class being optimized */
        uint64_t op1, op2;      /** Operand values analyzed */
    };

    /** Constructor */
    ComputationalSimplification(statistics::Group *parent);

    /** Destructor */
    ~ComputationalSimplification() = default;

    /**
     * Check if instruction is supported for computational simplification
     *
     * @param capability Operation class (IntMult, IntAlu, etc.)
     * @param instName Instruction mnemonic (e.g., "mul", "mla", "add")
     * @param numSrcs Number of source operands in the instruction
     * @return true if instruction is known and supported
     */
    bool isInstructionSupported(OpClass capability, const std::string& instName, int numSrcs);

    /**
     * Analyze instruction for computational simplification opportunities
     *
     * @param fuPool Functional unit pool for FU allocation
     * @param capability Operation class (IntMult, IntAlu, etc.)
     * @param instName Instruction mnemonic (e.g., "mul", "mla", "add")
     * @param op1 First operand value
     * @param op2 Second operand value
     * @return SimplificationResult with analysis and FU allocation
     */
    SimplificationResult analyzeInstruction(FUPool* fuPool, OpClass capability,
                                           const std::string& instName,
                                           uint64_t op1, uint64_t op2);

    /**
     * Check if operands allow multiplication fast-path
     * Detects: multiply by 0, 1, or powers of 2
     */
    bool isTrivialMultiply(uint64_t op1, uint64_t op2) const;

    /**
     * Check if operands allow ALU fast-path
     * Detects: add/subtract by 0
     */
    bool isTrivialAlu(uint64_t op1, uint64_t op2) const;

    /**
     * Mark functional unit as using fast-path execution
     * This tracks which FUs are executing with reduced latency
     */
    void setFastPathExecution(int fu_idx, bool is_fast_path);

    /**
     * Check if functional unit is using fast-path execution
     */
    bool isFastPathExecution(int fu_idx) const;

    /**
     * Clear fast-path execution state (called when FU is freed)
     */
    void clearFastPathExecution(int fu_idx);

    /**
     * Get statistics for computational simplification
     */
    struct CompSimpStats : public statistics::Group
    {
        CompSimpStats(statistics::Group *parent);

        /** Number of multiplication instructions using fast-path execution */
        statistics::Scalar fastPathMultiplications;

        /** Number of multiplication instructions using normal execution */
        statistics::Scalar normalMultiplications;

        /** Number of ALU instructions using fast-path execution */
        statistics::Scalar fastPathAlu;

        /** Number of ALU instructions using normal execution */
        statistics::Scalar normalAlu;

        /** Total cycles saved due to computational simplification */
        statistics::Scalar cyclesSavedCompSimp;
    };

    /** Get reference to statistics */
    CompSimpStats& getStats() { return stats; }

  private:
    /** Track which functional units are using fast-path execution */
    std::vector<bool> fastPathExecution;

    /** Known ARM multiplication instruction mnemonics for validation */
    static const std::set<std::string> knownIntMultInstructions;

    /** Known ARM ALU instruction mnemonics for validation (add/subtract only) */
    static const std::set<std::string> knownIntAluInstructions;

    /** Statistics for performance tracking */
    CompSimpStats stats;

    /**
     * Update statistics for computational simplification usage
     */
    void updateStats(OpClass capability, bool used_fast_path, Cycles cycles_saved);

    /**
     * Resize internal tracking vectors based on number of FUs
     */
    void resizeTracking(size_t num_fus);
};

} // namespace o3
} // namespace gem5

#endif // __CPU_O3_COMP_SIMP_HH__