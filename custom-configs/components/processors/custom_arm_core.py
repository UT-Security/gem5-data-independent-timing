"""
This is a modified version of the gem5 stdlib SimpleCore
in gem5/src/python/gem5/components/processors/simple_core.py
This version assumes X86, and adds an optional constructor parameter
for the specific CPU class to instantiate.

(Standard SimpleCore doesn't comprehend customized CPU models, always
creating an object of type 'ISA''TypeCPU', e.g., X86O3CPU)

NOTE:
This _should_ be a child class of BaseCPUCore, but SwitchableProcessor
in the stdlib (erroneously IMO) is constructed with a list of SimpleCore
objects rather than BaseCPUCore objects.  Therefore, pending a fix to that,
this has to be a child of SimpleCore rather than a sibling of it!
"""
from typing import Type
import importlib

from m5.objects import BaseCPU
from gem5.components.processors.simple_core import SimpleCore
from gem5.components.processors.base_cpu_core import BaseCPUCore
from gem5.components.processors.cpu_types import CPUTypes
from gem5.isas import ISA
import util.simarglib as simarglib

class CustomARMCore(SimpleCore):
    def __init__(
        self,
        core_id: int,
        core_type: CPUTypes,
        CPUCls: Type[BaseCPU] = None
    ) -> None:
        if CPUCls is None:
            if core_type == CPUTypes.ATOMIC:
                CPUCls = getattr(
                    importlib.import_module("m5.objects.ArmCPU"), "ArmAtomicSimpleCPU"
                )
            elif core_type == CPUTypes.O3:
                CPUCls = getattr(
                    importlib.import_module("m5.objects.ArmCPU"), "ArmO3CPU"
                )
            elif core_type == CPUTypes.TIMING:
                CPUCls = getattr(
                    importlib.import_module("m5.objects.ArmCPU"), "ArmTimingSimpleCPU"
                )
            elif core_type == CPUTypes.KVM:
                CPUCls = getattr(
                    # For some reason, the KVM CPU is under "m5.objects" not
                    # "m5.objects.{ISA}CPU".
                    importlib.import_module("m5.objects"), "ArmKvmCPU"
                )
            elif core_type == CPUTypes.MINOR:
                CPUCls = getattr(
                    importlib.import_module("m5.objects.ArmCPU"), "ArmMinorCPU"
                )
            else:
                raise NotImplementedError(
                    f"Unsupported CPUType '{core_type.name}'"
                )

        # Create the CPU object
        cpu_obj = CPUCls(cpu_id = core_id)


        # Don't call super()'s init, we want the grandparent init (see comment at top)!
        super(SimpleCore, self).__init__(
            # create object of specified CPU type
            # (either the default for cpu_type, or a custom class given in constructor)
            core = cpu_obj,
            isa = ISA.ARM
        )

        self._cpu_type = core_type

    def cpu_simobject_factory(cls, cpu_type: CPUTypes, isa: ISA, core_id: int):
        raise NotImplementedError(
            f"@classmethod cpu_simobject_factory() is not"
            "implemented in child class CustomX86Core"
        )
