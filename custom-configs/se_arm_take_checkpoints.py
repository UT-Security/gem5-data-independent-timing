"""
Sample SE config script to simulate an arbitrary program and its arguments
on the O3 Skylake CPU with a 3-level classic cache hierarchy
"""

import time
import argparse
from pathlib import Path

from m5.objects import *
from gem5.utils.requires import requires
from gem5.components.memory import DualChannelDDR4_2400
from gem5.isas import ISA
from gem5.components.processors.cpu_types import CPUTypes
from gem5.simulate.simulator import Simulator
from gem5.resources.resource import SimpointDirectoryResource

from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.processors.simple_processor import SimpleProcessor
import util.simarglib as simarglib
from workloads.se.custom_binary import CustomBinarySE

from gem5.simulate.exit_event import ExitEvent
from gem5.simulate.exit_event_generators import save_checkpoint_generator

# Create a processor
requires(isa_required=ISA.ARM)

# Add simpoint-specific arguments to simarglib before parsing
simpoint_group = simarglib.add_parser(
    "SimPoint Checkpointing",
    "Options for taking SimPoint-based checkpoints"
)

simpoint_group.add_argument(
    "--checkpoint-path",
    type=str,
    required=False,
    default="se_checkpoint_folder/",
    help="The directory to store the checkpoint.",
)

simpoint_group.add_argument(
    "--take-simpoint-checkpoints",
    action="store",
    type=str,
    required=True,
    help="<simpoint file,weight file,interval-length,warmup-length>",
)

# Parse all command-line args (now including simpoint arguments)
args_dict = simarglib.parse()
args = argparse.Namespace(**args_dict)

# Create a cache hierarchy
cache_hierarchy = NoCache()

# Create some DRAM
memory = DualChannelDDR4_2400(size="8GiB")

processor = SimpleProcessor(
    cpu_type=CPUTypes.ATOMIC,
    isa=ISA.ARM,
    # SimPoints only works with one core
    num_cores=1,
)

board = SimpleBoard(
    clk_freq="3GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

# Set up the workload
workload = CustomBinarySE()

# Set up the simpoint workload
simpoint_file_path, weight_file_path, interval_length, warmup_length = args.take_simpoint_checkpoints.split(",")

# Extract directory and filenames
simpoint_dir = str(Path(simpoint_file_path).parent)
simpoint_filename = Path(simpoint_file_path).name
weight_filename = Path(weight_file_path).name

# Use SimpointDirectoryResource to automatically load simpoint and weight files
simpoint_resource = SimpointDirectoryResource(
    local_path=simpoint_dir,
    simpoint_file=simpoint_filename,
    weight_file=weight_filename,
    simpoint_interval=int(interval_length),
    warmup_interval=int(warmup_length),
)

# Get binary and arguments from workload parameters
workload_params = workload.get_parameters()
board.set_se_simpoint_workload(
    binary=workload_params["binary"],
    arguments=workload_params["arguments"],
    simpoint=simpoint_resource,
)

dir = Path(args.checkpoint_path)

simulator = Simulator(
    board=board,
    on_exit_event={
        # using the SimPoints event generator in the standard library to take
        # checkpoints
        ExitEvent.SIMPOINT_BEGIN: save_checkpoint_generator(dir)
    },
)

# Run the simulation
starttime = time.time()

simulator.run()

totaltime = time.time() - starttime
print(
    f"***Exiting @ tick {simulator.get_current_tick()} because {simulator.get_last_exit_event_cause()}."
)
print(f"Total wall clock time: {totaltime:.2f} s = {(totaltime/60):.2f} min")
