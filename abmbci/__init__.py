from warnings import warn
from . import utils

try:
    from .abm import ABM
    import abmbciext as abm
except ModuleNotFoundError:
    warn("ABM software not available switching to mock system")
    from . import mock_abm as abm
    # zApjav-ropra7-zazwoh

