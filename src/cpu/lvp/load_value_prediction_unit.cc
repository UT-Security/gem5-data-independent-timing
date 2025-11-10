/*
 * Robert Viramontes
 * Created March 26, 2021
 * Based on the learning/part2/simpleobject
 *
 * Ported to modern gem5 (v25.0+) with namespace updates
 */

#include "cpu/lvp/load_value_prediction_unit.hh"

#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/LVP.hh"

namespace gem5
{

LoadValuePredictionUnit::LoadValuePredictionUnit(const LoadValuePredictionUnitParams &params) :
    SimObject(params),
    loadClassificationTable(params.load_classification_table),
    loadValuePredictionTable(params.load_value_prediction_table),
    lvpStats(this)
{
    DPRINTF(LVP, "Created the LVP (CVU disabled)\n");
    panic_if(!loadClassificationTable, "LVP must have a non-null LCT");
    panic_if(!loadValuePredictionTable, "LVP must have a non-null LVPT");
}

LoadValuePredictionUnit::LoadValuePredictionUnitStats::LoadValuePredictionUnitStats(LoadValuePredictionUnit *lvp)
    : statistics::Group(lvp),
      ADD_STAT(numUnpredictableLoads, statistics::units::Count::get(),
               "Number of loads classified as unpredictable"),
      ADD_STAT(numConstLoads, statistics::units::Count::get(),
               "Number of loads classified as constant"),
      ADD_STAT(numConstLoadsMispredicted, statistics::units::Count::get(),
               "Number of constant loads incorrectly predicted"),
      ADD_STAT(numConstLoadsCorrect, statistics::units::Count::get(),
               "Number of constant loads correctly predicted"),
      ADD_STAT(totalLoads, statistics::units::Count::get(),
               "Total loads processed by the Load value predictor"),
      ADD_STAT(numZeroConstLoads, statistics::units::Count::get(),
               "Number of constant loads with value 0"),
      ADD_STAT(numOneConstLoads, statistics::units::Count::get(),
               "Number of constant loads with value 1"),
      ADD_STAT(numSquashes, statistics::units::Count::get(),
               "Number of pipeline squashes due to LVP mispredictions"),
      ADD_STAT(constAccuracy, statistics::units::Ratio::get(),
               "Constant load prediction accuracy"),
      ADD_STAT(predictionCoverage, statistics::units::Ratio::get(),
               "Percentage of all loads that are predicted (constant loads)")
{
    constAccuracy = numConstLoadsCorrect / numConstLoads;
    predictionCoverage = numConstLoads / totalLoads;
}

LvptResult
LoadValuePredictionUnit::lookup(ThreadID tid, Addr inst_addr)
{
    lvpStats.totalLoads++;
    bool lvptResultValid = false;
    auto lvptResult = loadValuePredictionTable->lookup(tid, inst_addr, &lvptResultValid);

    // Always consult LCT for classification - LVPT validity only affects whether
    // we can use the predicted value, not whether we classify the load
    auto lctResult = loadClassificationTable->lookup(tid, inst_addr);

    LvptResult result;

    result.taken = lctResult;
    result.value = 0;
    if(lvptResultValid) {
        result.value = lvptResult;
    }

    if(lctResult == LVP_CONSTANT)
    {
        DPRINTF(LVP, "Constant load for thread %d at address %#x had value %d\n",
            tid, inst_addr, lvptResult);
        if(lvptResult == 0) lvpStats.numZeroConstLoads++;
        else if(lvptResult == 1) lvpStats.numOneConstLoads++;
    }

    // Stat collection based on LCT classification
    if(lctResult == LVP_CONSTANT)
        lvpStats.numConstLoads++;
    else
        lvpStats.numUnpredictableLoads++;

    return result;
}

bool
LoadValuePredictionUnit::verifyPrediction(ThreadID tid, Addr pc, Addr load_address,
                                    RegVal correct_val, RegVal predicted_val,
                                    LVPType classification) {
    /**
     * LVPT: lvpt::update(pc, tid, correct_val)
     * LCT:  lct::update(pc, tid) retval lctResult
     * CVU: if(lctResult = constant) updateCVU(pc, tid, pc[9:2], load_address);
     */

    if(classification == LVP_CONSTANT) {
        if(predicted_val != correct_val) {
            lvpStats.numConstLoadsMispredicted++;
        }
        else {
            lvpStats.numConstLoadsCorrect++;
        }
    }

    loadValuePredictionTable->update(pc, correct_val, tid);
    auto result = loadClassificationTable->update(tid, pc, classification, predicted_val == correct_val);
    if(result == LVP_CONSTANT) {
        DPRINTF(LVP, "[TID: %d] Load instruction 0x%x marked constant by LCT (CVU disabled)\n", tid, pc);
        // CVU removed - constant loads no longer tracked separately
    }
    return true;
}

std::pair<LVPType, RegVal>
LoadValuePredictionUnit::predictLoad(ThreadID tid, Addr pc) {
    DPRINTF(LVP, "Load Instruction: 0x%x being processed by LVPU\n", pc);
    std::pair<LVPType, RegVal> temp;
    LvptResult result = this->lookup(tid, pc);
    temp.first = result.taken;
    temp.second = result.value;
    DPRINTF(LVP, "Load Instruction: 0x%x predicted as type %d with value %d\n",
            pc, temp.first, temp.second);
    return temp;
}

Addr
LoadValuePredictionUnit::lookupLVPTIndex(ThreadID tid, Addr pc) {
    return loadValuePredictionTable->getIndex(pc, tid);
}

void
LoadValuePredictionUnit::startup()
{
    // Before simulation starts, we need to schedule the event
    DPRINTF(LVP, "Starting the LVP");
}

} // namespace gem5