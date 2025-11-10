"""
Sample SE config script to simulate an arbitrary program and its arguments
on the O3 Skylake CPU with a 3-level classic cache hierarchy
"""

import time
from typing import Final

import m5
from m5.objects import *
from gem5.utils.requires import requires
from gem5.components.memory import DualChannelDDR4_2400
from gem5.isas import ISA
from gem5.components.processors.cpu_types import CPUTypes
from gem5.simulate.simulator import Simulator
from components.processors.custom_arm_switchable_processor import CustomARMSwitchableProcessor
from components.processors.custom_arm_processor import CustomARMProcessor
from components.cpus.O3_ARM_v8 import O3_ARM_v8_3
from termcolor import colored, cprint

from components.cache_hierarchies.three_level_classic import ThreeLevelClassicHierarchy
import util.simarglib as simarglib
from workloads.se.custom_binary import CustomBinarySE

from util.event_managers.event_manager import EventCoordinator
from util.event_managers.roi.periodic import PeriodicROIManager

from components.boards.custom_simple_board import CustomSimpleBoard
from gem5.utils.requires import requires

# Parse all command-line args
simarglib.parse()


# Create a processor
requires(isa_required=ISA.ARM)

# O3 core type recommended
cpu_class = O3_ARM_v8_3

processor = CustomARMSwitchableProcessor(SwitchCPUCls=cpu_class)

# Create a cache hierarchy
cache_hierarchy = ThreeLevelClassicHierarchy()

# Create some DRAM
memory = DualChannelDDR4_2400(size="3GiB")

# Create a board
board = CustomSimpleBoard(
    processor=processor, memory=memory, cache_hierarchy=cache_hierarchy,
)

# Set up the workload
workload = CustomBinarySE()
board.set_workload(workload)


roi_manager = PeriodicROIManager()
coordinator = EventCoordinator([roi_manager])

# Set up the simulator
simulator = Simulator(
    board=board,
    on_exit_event=coordinator.get_event_handlers(),
)
coordinator.register(simulator)

# Print information
print(
    colored(
        "***Fast-forward interval:",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{roi_manager._ff_interval:,} instructions",
        color="blue",
    ),
)
print(
    colored(
        "***Warmup interval      :",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{roi_manager._warmup_interval:,} instructions",
        color="blue",
    ),
)
print(
    colored(
        "***ROI interval         :",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{roi_manager._roi_interval:,} instructions",
        color="blue",
    ),
)
print(
    colored(
        "***Initial fast-forward :",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{roi_manager._init_ff_interval:,} instructions",
        color="blue",
    ),
)
print(
    colored(
        "***Maximum ROIs         :",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{roi_manager._num_rois or 'Unlimited'}",
        color="blue",
    ),
)
print(
    colored("***Continue simulation  :", color="blue", attrs=["bold"]),
    colored(
        f"{roi_manager._continue_sim}",
        color="blue",
    ),
)

# Run the simulation
start_wall_time: Final[float] = time.time()
cprint("***Beginning ARM simulation with ROI sampling!", color="blue", attrs=["bold"])

simulator.run()

elapsed_wall_time: Final[float] = time.time() - start_wall_time
elapsed_instructions = coordinator.get_current_time().instruction or 0
elapsed_ticks = simulator.get_current_tick()
print(
    colored(
        f"***Instruction {elapsed_instructions:,}, tick {elapsed_ticks:,}:",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"Exiting because {simulator.get_last_exit_event_cause()}.",
        color="blue",
    ),
)
print(
    colored(
        "***Total wall clock time:",
        color="blue",
        attrs=["bold"],
    ),
    colored(
        f"{(elapsed_wall_time/60):.2f} min",
        color="blue",
    ),
)
