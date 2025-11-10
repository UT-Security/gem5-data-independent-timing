"""
Sample SE config script to simulate an arbitrary program and its arguments
on the O3 Skylake CPU with a 3-level classic cache hierarchy
"""

import time

import m5
from m5.objects import *
from gem5.utils.requires import requires
from gem5.components.memory import DualChannelDDR4_2400
from gem5.isas import ISA
from gem5.components.processors.cpu_types import CPUTypes
from gem5.simulate.simulator import Simulator

from components.boards.custom_simple_board import CustomSimpleBoard
from components.cpus.O3_ARM_v8 import O3_ARM_v8_3
from components.processors.custom_arm_processor import CustomARMProcessor
from components.cache_hierarchies.three_level_classic import ThreeLevelClassicHierarchy
import util.simarglib as simarglib
from workloads.se.custom_binary import CustomBinarySE

# Parse all command-line args
simarglib.parse()

# Create a processor
requires(isa_required=ISA.ARM)

# O3 core type recommended
cpu_class = O3_ARM_v8_3

processor = CustomARMProcessor(CPUCls=cpu_class)

# Create a cache hierarchy
cache_hierarchy = ThreeLevelClassicHierarchy()

# Create some DRAM
memory = DualChannelDDR4_2400(size="3GiB")

# Create a board
board = CustomSimpleBoard(
    processor=processor, cache_hierarchy=cache_hierarchy, memory=memory
)

# Set up the workload
workload = CustomBinarySE()
board.set_workload(workload)


# Set up the simulator
simulator = Simulator(board=board)

# Run the simulation
starttime = time.time()

simulator.run()

totaltime = time.time() - starttime
print(
    f"***Exiting @ tick {simulator.get_current_tick()} because {simulator.get_last_exit_event_cause()}."
)
print(f"Total wall clock time: {totaltime:.2f} s = {(totaltime/60):.2f} min")
