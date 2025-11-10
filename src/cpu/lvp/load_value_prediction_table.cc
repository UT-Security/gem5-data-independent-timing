/****************************************************************************/
// Author 	: Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 752
//
// Ported to modern gem5 (v25.0+) with namespace updates
/****************************************************************************/

#include "cpu/lvp/load_value_prediction_table.hh"

#include "base/intmath.hh"
#include "base/trace.hh"
#include "debug/LVPT.hh"

namespace gem5
{

LoadValuePredictionTable::LoadValuePredictionTable(const LoadValuePredictionTableParams &params)
    : SimObject(params),
      numEntries(params.entries),
      historyDepth(params.historyDepth),
      instShiftAmt(params.instShiftAmt)
{
    DPRINTF(LVPT, "LVPT: Creating LVPT object.\n");

    if (!isPowerOf2(numEntries)) {
        fatal("LVPT entries is not a power of 2!");
    }

    LVPT.resize(numEntries);

    DPRINTF(LVPT, "LVPT: Doing an initial reset \n");
    for (unsigned i = 0; i < numEntries; ++i) {
        LVPT[i].valid = false;
    }

    idxMask = numEntries - 1;
    log2NumThreads = 0;  // Assume single thread; can be made configurable later

    std::cout << "LVPT: Created LVPT with " << numEntries
              << " entries, index mask: 0x" << std::hex << idxMask << std::dec
              << ", instShiftAmt: " << instShiftAmt
              << ", log2NumThreads: " << log2NumThreads
              << " (no tags - constructive aliasing enabled)"
              << std::endl;
}

/* Reset API */
void
LoadValuePredictionTable::reset()
{
    for (unsigned i = 0; i < numEntries; ++i) {
        LVPT[i].valid = false;
    }
}

/* APIs to get index and tag*/
unsigned
LoadValuePredictionTable::getIndex(Addr instPC, ThreadID tid)
{
    // Need to shift PC over by the word offset.
    // Math: ((instPC >> instShiftAmt)^(tid<<(tagShiftAmt-instShiftAmt-log2NumThreads)))&idxMask;
    return (instPC >> instShiftAmt) & idxMask;
}

/** Checks if the load entry is in the LVPT **/
bool
LoadValuePredictionTable::valid(Addr instPC, ThreadID tid)
{
    unsigned LVPT_idx = getIndex(instPC, tid);

    assert(LVPT_idx < numEntries);

    // Check if: (a) LVPT entry is valid
    // (b) tid matches
    // No tag checking - allows constructive aliasing
    if (LVPT[LVPT_idx].valid && LVPT[LVPT_idx].tid == tid) {
        return true;
    } else {
        return false;
    }
}

// data = 0 represent invalid entry.
RegVal
LoadValuePredictionTable::lookup(ThreadID tid, Addr instPC, bool *lvptResultValid)
{
    unsigned LVPT_idx = getIndex(instPC, tid);

    assert(LVPT_idx < numEntries);

    if (valid(instPC, tid)) {
        DPRINTF(LVPT, "Found valid entry for tid: %d at pc %#x : %d \n",
            tid, instPC, LVPT[LVPT_idx].target);
        *lvptResultValid = true;
        return LVPT[LVPT_idx].target;
    } else {
        // DPRINTF(LVPT, "Did not find valid entry for tid: %d at address %#x \n",
        //     tid, instPC);
        *lvptResultValid = false;  // BUG FIX: Must set to false!
        return 0;
    }
}

void
LoadValuePredictionTable::update(Addr instPC, const RegVal target, ThreadID tid)
{
    unsigned LVPT_idx = getIndex(instPC, tid);
    DPRINTF(LVPT, "LVPT: Updating index %d with value %llu\n", LVPT_idx, target);

    assert(LVPT_idx < numEntries);

    LVPT[LVPT_idx].tid = tid;
    LVPT[LVPT_idx].valid = true;
    LVPT[LVPT_idx].target = target;
}

} // namespace gem5