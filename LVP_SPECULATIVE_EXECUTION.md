# Load Value Prediction - Speculative Execution Implementation

## Overview
This document describes the speculative execution implementation for Load Value Prediction (LVP) in gem5 O3 CPU.

## What Changed

### Previous Implementation (No Performance Benefit)
- Predicted load values but **never used them**
- Dependent instructions **stalled waiting** for load completion
- Only used predictions for verification and squashing
- Result: **Overhead with no speedup**

### New Implementation (Speculative Execution)
- Predicted load values are **written speculatively** to destination registers
- Destination registers **marked ready** in scoreboard
- Dependent instructions **execute immediately** using predicted values
- Result: **Actual performance improvement**

---

## Implementation Details

### File: `src/cpu/o3/iew.cc` (Lines 1021-1050)

**Location:** Load instruction dispatch in IEW stage

**What it does:**
1. Calls `predictLoad()` to get classification and predicted value
2. For PREDICTABLE and CONSTANT loads:
   - Writes predicted value using `inst->setRegOperand(staticInst.get(), 0, predicted_value)`
   - Marks register ready: `scoreboard->setReg(dest_reg)`
3. Dependent instructions can now execute using predicted value

**Code:**
```cpp
if (classification == LVP_PREDICTABLE || classification == LVP_CONSTANT) {
    // Write predicted value to destination register (speculative)
    inst->setRegOperand(inst->staticInst.get(), 0, predicted_value);

    // Mark register as ready in scoreboard
    scoreboard->setReg(dest_reg);
}
```

### File: `src/cpu/o3/lsq_unit.cc` (Lines 1097-1145)

**Location:** Load completion and verification

**What it does:**
1. `completeAcc()` overwrites register with actual loaded value
   - If prediction correct: overwrites with same value (no harm)
   - If prediction wrong: overwrites with correct value
2. Verifies prediction correctness
3. If misprediction: squashes all younger instructions
   - Removes instructions that executed with wrong predicted value
   - They will re-execute with correct value

**Code:**
```cpp
inst->completeAcc(pkt);  // Writes actual value, overwrites predicted value

bool correct = inst->verifyPrediction(inst->threadNumber, actual_value);

if (!correct && (inst->isConstantLoad() || inst->isPredictableLoad())) {
    // Squash younger instructions that used wrong value
    iewStage->squashDueToLoadValueMispred(inst, inst->threadNumber);
    cpu->lvp->recordSquash();
}
```

---

## Execution Flow

### Scenario 1: Correct Prediction (Common Case ~90%+)

```
Time  Event
----  -----
t0    Load R3, [R1] enters IEW
      - Predict: R3 will be 0x42
      - Write 0x42 to physical register P42
      - Mark P42 READY in scoreboard

t1    Add R4, R3, R5 wakes up
      - Reads P42 (value: 0x42)
      - Executes immediately!
      - Writes result to P43

t2    Load completes from cache
      - Actual value: 0x42 (CORRECT!)
      - completeAcc overwrites P42 with 0x42 (same value)
      - No squash needed
      - Add R4 result is correct

Result: Add executed early, saved ~5-10 cycles!
```

### Scenario 2: Misprediction (Rare Case ~5-10%)

```
Time  Event
----  -----
t0    Load R3, [R1] enters IEW
      - Predict: R3 will be 0x42
      - Write 0x42 to physical register P42
      - Mark P42 READY

t1    Add R4, R3, R5 wakes up
      - Reads P42 (value: 0x42)
      - Executes with WRONG value
      - Writes wrong result to P43

t2    Load completes from cache
      - Actual value: 0x99 (WRONG!)
      - completeAcc overwrites P42 with 0x99
      - Detect misprediction
      - SQUASH Add and all younger instructions
      - Pipeline flushes

t3    Add R4, R3, R5 re-executes
      - Now reads P42 (value: 0x99)
      - Executes with CORRECT value
      - Writes correct result

Result: Penalty = squash overhead, but rare
```

---

## Performance Analysis

### When LVP Helps:
- **High accuracy** (>90%): Common case benefits outweigh rare mispredictions
- **Long load latency**: More cycles saved by early execution
- **Dependent instruction chains**: More instructions execute early

### When LVP Hurts:
- **Low accuracy** (<70%): Too many squashes
- **Vector-heavy workloads**: LVP only works on integer loads
- **Short load latency**: Less benefit from speculation

### Expected Speedup:
For integer-heavy workloads with predictable loads:
- **IPC improvement**: 5-15%
- **Load latency hiding**: 50-70% of predicted loads
- **Squash overhead**: <5% with good accuracy

---

## Statistics to Monitor

From `m5out/stats.txt`:

```
system.cpu.lvp.numConstLoads              # Constant loads found
system.cpu.lvp.numPredictableLoads        # Predictable loads found
system.cpu.lvp.numConstLoadsCorrect       # Correct predictions
system.cpu.lvp.numSquashes                # Pipeline squashes (NEW!)
system.cpu.lvp.predictionAccuracy         # Overall accuracy

# Key metrics:
Coverage = (numConstLoads + numPredictableLoads) / totalLoads
Accuracy = numConstLoadsCorrect / numConstLoads
Squash Rate = numSquashes / totalLoads
```

---

## Testing

### Compile:
```bash
scons build/ARM/gem5.opt -j$(nproc)
```

### Run with LVP enabled:
Uncomment lines 194-210 in `custom-configs/components/cpus/O3_ARM_v8.py`

### Compare:
- **Baseline**: LVP disabled (lines commented)
- **LVP**: LVP enabled (lines uncommented)

### Metrics to compare:
- `system.cpu.numCycles` - Should decrease with LVP
- `system.cpu.ipc` - Should increase with LVP
- `system.cpu.lvp.numSquashes` - Should be low (<5% of loads)

---

## Notes

1. **Only for IntRegClass**: Vector/FP loads not supported yet
2. **Scoreboard integration**: Uses existing scoreboard for wakeup
3. **ROB-based recovery**: Squashing uses existing ROB mechanism
4. **Same as branch prediction**: Similar speculative execution model

## Future Enhancements

1. Support vector register loads
2. Confidence-based prediction (don't predict if low confidence)
3. Per-load prediction history
4. Integration with prefetcher
