"""
Library implementing shared argparsing between modules, with each
module able to add its own argument group.  If arguments conflict,
bad things will happen.  No attempt is made to fix this! :-)
Based on: https://www.doc.ic.ac.uk/~nuric/coding/
argparse-with-multiple-files-to-handle-configuration-in-python.html
"""
import argparse
from typing import Dict, Any

# Global parser
parser = argparse.ArgumentParser("Gem5 Simulation Arguments")

# Add DIT-specific arguments
dit_group = parser.add_argument_group("DIT Options", "ARM Data Independent Timing options")
dit_group.add_argument("--dit-default", type=int, choices=[0, 1], default=0,
                      help="Default DIT bit value (0=optimizations enabled, 1=disabled)")
dit_group.add_argument("--dit-stats", action="store_true",
                      help="Enable DIT-specific statistics collection")
dit_group.add_argument("--dit-debug", action="store_true",
                      help="Enable DIT debug trace flags")

# Global table of parsed args
args: Dict[str, Any] = {}

def add_parser(group_name: str, description: str = ""):
    """ Add a module's argument group and return the group """
    return parser.add_argument_group(group_name, description)

def parse() -> Dict[str, Any]:
    """ Parse all collected arguments """
    args.update(vars(parser.parse_args()))
    return args

def get(key: str):
    return args.get(key)
