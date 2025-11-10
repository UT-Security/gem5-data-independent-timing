/****************************************************************************/
// Author 	: Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 752
//
// Ported to modern gem5 (v25.0+) with namespace updates
/****************************************************************************/

#ifndef __CPU_LVP_LOADVALUEPREDICTIONTABLE_HH__
#define __CPU_LVP_LOADVALUEPREDICTIONTABLE_HH__

#include "base/logging.hh"
#include "base/types.hh"
#include "cpu/static_inst.hh"
#include "params/LoadValuePredictionTable.hh"
#include "sim/sim_object.hh"

namespace gem5
{

struct LoadValuePredictionTableParams;

/** Creating a default Load Value Prediction Table entry
 *  which will have below attributes
 *  tag   : Specifies the Opcode of the Load instruction
 *  taget : Specifies the Load value associated with the tag
 *  valid : Specifies if the value loaded is valid
 */

class LoadValuePredictionTable : public SimObject
{
  private:
    struct LVPTEntry
    {
        LVPTEntry()
            : target(0), tid(0), valid(false)
        {}

        /** The entry's target. */
        RegVal target;

        /** The entry's thread id. */
        ThreadID tid;

        /** Whether or not the entry is valid. */
        bool valid;
    };

  public:
    /** Creates a LVPT with the given number of entries, number of bits per
     *  tag, and instruction offset amount.
     *  @param numEntries Number of entries for the LVPT.
     *  @param tagBits Number of bits for each tag in the LVPT.
     *  @param instShiftAmt Offset amount for instructions to ignore alignment.
     */
    LoadValuePredictionTable(const LoadValuePredictionTableParams &params);

    void reset();

    /** Looks up an address in the LVPT. Must call valid() first on the address.
     *  @param inst_PC The address of the branch to look up.
     *  @param tid The thread id.
     *  @return Returns the predicated load value.
     */
    RegVal lookup(ThreadID tid, Addr instPC, bool *lvptResultValid);

    /** Checks if the load entry is in the LVPT.
     *  @param inst_PC The address of the branch to look up.
     *  @param tid The thread id.
     *  @return Whether or not the branch exists in the LVPT.
     */
    bool valid(Addr instPC, ThreadID tid);

    /** Updates the LVPT with the latest predicted Load Value.
     *  @param inst_PC The address of the branch being updated.
     *  @param target_PC The predicted target data.
     *  @param tid The thread id.
     */

    void update(Addr instPC, const RegVal target, ThreadID tid);

    /** Returns the index into the LVPT, based on the branch's PC.
     *  @param inst_PC The branch to look up.
     *  @return Returns the index into the LVPT.
     */

    unsigned getIndex(Addr instPC, ThreadID tid);

  private:

    /** The actual LVPT declaration */
    std::vector<LVPTEntry> LVPT;

    /** The number of entries in the LVPT. */
    const unsigned numEntries;

    /** Depth of data history kept in the LVPT*/
    const unsigned historyDepth;

    /** The index mask. */
    unsigned idxMask;

    /** Number of bits to shift PC when calculating index. */
    unsigned instShiftAmt;

    /** Log2 NumThreads used for hashing threadid */
    unsigned log2NumThreads;
};

} // namespace gem5

#endif // __CPU_LVP_LOADVALUEPREDICTIONTABLE_HH__