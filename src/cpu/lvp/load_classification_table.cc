/*
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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

#include "cpu/lvp/load_classification_table.hh"

#include <iostream>

#include "base/intmath.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/LCT.hh"

namespace gem5
{

LoadClassificationTable::LoadClassificationTable(const LoadClassificationTableParams &params)
    : SimObject(params),
      numEntries(params.numEntries),
      localCtrBits(params.localCtrBits),
      localCtrs(numEntries, SatCounter8(localCtrBits, 0)),
      indexMask(numEntries - 1),
      instShiftAmt(2),
      invalidateConstToZero(params.invalidateConstToZero)
{
    if (!isPowerOf2(numEntries)) {
        fatal("LCT numEntries is not a power of 2!");
    }

    std::cout << "LCT: Created with " << numEntries << " counters"
              << ", index mask: 0x" << std::hex << indexMask << std::dec
              << ", counter bits: " << localCtrBits
              << ", instShiftAmt: " << instShiftAmt
              << " (no tags, no TID - full constructive aliasing enabled)"
              << std::endl;
}

LVPType
LoadClassificationTable::lookup(ThreadID tid, Addr inst_addr)
{
    unsigned local_predictor_idx = getLocalIndex(inst_addr);
    uint8_t counter_val = localCtrs[local_predictor_idx];

    DPRINTF(LCT, "LCT lookup: index %#x, counter value %i\n",
            local_predictor_idx, (int)counter_val);

    return getPrediction(counter_val);
}

LVPType
LoadClassificationTable::update(ThreadID tid, Addr inst_addr, LVPType prediction, bool prediction_correct)
{
    unsigned local_predictor_idx = getLocalIndex(inst_addr);

    // Threshold-based update:
    // - Correct prediction: increment counter (saturates at max)
    // - Incorrect prediction: reset counter to 0 or decrement counter
    if (prediction_correct) {
        DPRINTF(LCT, "LCT update: index %#x correct, incrementing counter (now %d)\n",
                local_predictor_idx, (int)localCtrs[local_predictor_idx] + 1);
        localCtrs[local_predictor_idx]++;  // Saturates at max automatically
    } else {
        if(invalidateConstToZero) {
            if(localCtrs[local_predictor_idx] >= CONSTANT_THRESHOLD) {
                DPRINTF(LCT, "LCT update: index %#x incorrect constant, resetting counter to 0\n", local_predictor_idx);
                resetCtr(local_predictor_idx);  // Reset to 0
            }
        } else {
            DPRINTF(LCT, "LCT update: index %#x incorrect, resetting counter to 0\n", local_predictor_idx);
            localCtrs[local_predictor_idx]--;  // Reset to 0
        }
    }

    uint8_t counter_val = localCtrs[local_predictor_idx];
    DPRINTF(LCT, "LCT update: index %#x counter now %d, classification: %s\n",
            local_predictor_idx, (int)counter_val,
            (counter_val >= CONSTANT_THRESHOLD) ? "CONSTANT" : "UNPREDICTABLE");
    return getPrediction(counter_val);
}

void
LoadClassificationTable::reset()
{
    for (unsigned c = 0; c < numEntries; c++)
    {
        localCtrs[c] = SatCounter8(localCtrBits, 0);
    }
}

void
LoadClassificationTable::resetCtr(unsigned local_predictor_idx)
{
    while (localCtrs[local_predictor_idx] != LVP_STRONG_UNPREDICTABLE) {
        localCtrs[local_predictor_idx]--;
    }
}

inline
LVPType
LoadClassificationTable::getPrediction(uint8_t &count)
{
    // Simple threshold: if counter >= 30, classify as CONSTANT
    // Otherwise, classify as UNPREDICTABLE
    return (count >= CONSTANT_THRESHOLD) ? LVP_CONSTANT : LVP_STRONG_UNPREDICTABLE;
}

inline
unsigned
LoadClassificationTable::getLocalIndex(Addr &inst_addr)
{
    return (inst_addr >> instShiftAmt) & indexMask;
}

} // namespace gem5