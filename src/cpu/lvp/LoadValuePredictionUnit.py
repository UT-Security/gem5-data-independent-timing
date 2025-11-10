# -*- coding: utf-8 -*-
# Robert Viramontes
# Created: March 25, 2021
# Based on the learning/part2/simpleobject
# Updated for modern gem5 (v25.0+)

from m5.params import *
from m5.SimObject import SimObject

class LoadClassificationTable(SimObject):
    type = 'LoadClassificationTable'
    cxx_header = "cpu/lvp/load_classification_table.hh"
    cxx_class = 'gem5::LoadClassificationTable'

    numEntries = Param.Unsigned(1024, "Number of counter entries")
    localCtrBits = Param.Unsigned(6, "Bits per counter (6 bits = max 63, threshold is 30)")
    invalidateConstToZero = Param.Bool(True, "Reset counter to 0 on misprediction (always true for threshold)")
    # direct mapped, no tags (allows constructive aliasing)

class LoadValuePredictionTable(SimObject):
    type = 'LoadValuePredictionTable'
    cxx_header = "cpu/lvp/load_value_prediction_table.hh"
    cxx_class = 'gem5::LoadValuePredictionTable'

    entries = Param.Unsigned(4096, "Number of entries in the prediction table")
    historyDepth = Param.Unsigned(1, "History depth")
    instShiftAmt = Param.Unsigned(2, "Number of bits to shift PC (to ignore alignment)")
    #Direct mapped, no tags (allows constructive aliasing)

class ConstantVerificationUnit(SimObject):
    type = 'ConstantVerificationUnit'
    cxx_header = "cpu/lvp/constant_verification_unit.hh"
    cxx_class = 'gem5::ConstantVerificationUnit'

    entries = Param.Unsigned(8, "Number of entries in the CVU CAM")
    replacementPolicy = Param.Unsigned(1, "Replacement policy of the fully-assoc CAM")

class LoadValuePredictionUnit(SimObject):
    type = 'LoadValuePredictionUnit'
    cxx_header = "cpu/lvp/load_value_prediction_unit.hh"
    cxx_class = 'gem5::LoadValuePredictionUnit'

    load_classification_table = Param.LoadClassificationTable(LoadClassificationTable(), "A load classification table")
    load_value_prediction_table = Param.LoadValuePredictionTable(LoadValuePredictionTable(), "A load value prediction table")
    # constant_verification_unit = Param.ConstantVerificationUnit(ConstantVerificationUnit(), "A constant verification unit")  # CVU removed