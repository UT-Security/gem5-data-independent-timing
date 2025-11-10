/*
 * Load Value Prediction Oracle - Headroom Study
 * Tracks actual load behavior to determine maximum predictability
 */

#ifndef __CPU_O3_LOAD_ORACLE_HH__
#define __CPU_O3_LOAD_ORACLE_HH__

#include <iostream>
#include <map>
#include "base/types.hh"
#include "cpu/reg_class.hh"

namespace gem5
{
namespace o3
{

class LoadOracle
{
  private:
    struct LoadInfo {
        uint64_t totalExecs = 0;
        std::map<RegVal, uint64_t> valueHistogram;
    };

    std::map<Addr, LoadInfo> loadTracker;

    // Track loads by register class
    std::map<RegClassType, uint64_t> loadsByRegClass;

  public:
    LoadOracle() {}

    // Track a load execution
    void recordLoad(Addr pc, RegVal value, RegClassType regClass) {
        // Only track detailed values for integer loads
        if (regClass == IntRegClass) {
            loadTracker[pc].totalExecs++;
            loadTracker[pc].valueHistogram[value]++;
        }

        // Count all loads by register class
        loadsByRegClass[regClass]++;
    }

    // Compute and print oracle statistics
    void printStats() {
        uint64_t totalLoadPCs = loadTracker.size();
        uint64_t constantPCs = 0;
        uint64_t unpredictablePCs = 0;

        uint64_t totalExecs = 0;
        uint64_t constantExecs = 0;
        uint64_t unpredictableExecs = 0;

        for (const auto& [pc, info] : loadTracker) {
            totalExecs += info.totalExecs;

            // Find most common value
            uint64_t maxCount = 0;
            for (const auto& [val, count] : info.valueHistogram) {
                if (count > maxCount) maxCount = count;
            }

            double predictability = (double)maxCount / info.totalExecs;

            if (predictability == 1.0) {
                constantPCs++;
                constantExecs += info.totalExecs;
            } else {
                unpredictablePCs++;
                unpredictableExecs += info.totalExecs;
            }
        }

        // Count total loads across all register classes
        uint64_t totalAllLoads = 0;
        for (const auto& [regClass, count] : loadsByRegClass) {
            totalAllLoads += count;
        }

        std::cout << "\n========== LOAD ORACLE HEADROOM STUDY ==========\n";
        std::cout << "Total Unique Load PCs: " << totalLoadPCs << " (IntReg only)\n";
        std::cout << "Total Load Executions: " << totalExecs << " (IntReg only)\n\n";

        std::cout << "=== Loads by Register Class ===\n";
        if (loadsByRegClass.find(IntRegClass) != loadsByRegClass.end()) {
            std::cout << "IntRegClass:     " << loadsByRegClass.at(IntRegClass)
                      << " (" << (100.0*loadsByRegClass.at(IntRegClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(FloatRegClass) != loadsByRegClass.end()) {
            std::cout << "FloatRegClass:   " << loadsByRegClass.at(FloatRegClass)
                      << " (" << (100.0*loadsByRegClass.at(FloatRegClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(VecRegClass) != loadsByRegClass.end()) {
            std::cout << "VecRegClass:     " << loadsByRegClass.at(VecRegClass)
                      << " (" << (100.0*loadsByRegClass.at(VecRegClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(VecElemClass) != loadsByRegClass.end()) {
            std::cout << "VecElemClass:    " << loadsByRegClass.at(VecElemClass)
                      << " (" << (100.0*loadsByRegClass.at(VecElemClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(VecPredRegClass) != loadsByRegClass.end()) {
            std::cout << "VecPredRegClass: " << loadsByRegClass.at(VecPredRegClass)
                      << " (" << (100.0*loadsByRegClass.at(VecPredRegClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(CCRegClass) != loadsByRegClass.end()) {
            std::cout << "CCRegClass:      " << loadsByRegClass.at(CCRegClass)
                      << " (" << (100.0*loadsByRegClass.at(CCRegClass)/totalAllLoads) << "%)\n";
        }
        if (loadsByRegClass.find(MiscRegClass) != loadsByRegClass.end()) {
            std::cout << "MiscRegClass:    " << loadsByRegClass.at(MiscRegClass)
                      << " (" << (100.0*loadsByRegClass.at(MiscRegClass)/totalAllLoads) << "%)\n";
        }
        std::cout << "Total All Loads: " << totalAllLoads << "\n\n";

        std::cout << "=== Integer Load Predictability (Static) ===\n";
        std::cout << "Constant (100%):       " << constantPCs
                  << " (" << (100.0*constantPCs/totalLoadPCs) << "%)\n";
        std::cout << "Unpredictable (<100%): " << unpredictablePCs
                  << " (" << (100.0*unpredictablePCs/totalLoadPCs) << "%)\n\n";

        std::cout << "=== Integer Load Predictability (Dynamic) ===\n";
        std::cout << "Constant (100%):       " << constantExecs
                  << " (" << (100.0*constantExecs/totalExecs) << "%)\n";
        std::cout << "Unpredictable (<100%): " << unpredictableExecs
                  << " (" << (100.0*unpredictableExecs/totalExecs) << "%)\n";
        std::cout << "================================================\n\n";
    }
};

} // namespace o3
} // namespace gem5

#endif // __CPU_O3_LOAD_ORACLE_HH__
