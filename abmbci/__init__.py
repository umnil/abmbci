from warnings import warn
from . import utils
from .trigger import Trigger

try:
    from abmbci._abmbci import Headset, HeadsetType
except ModuleNotFoundError:
    warn("ABM software not available switching to mock system")
    from .mock_abm import Headset, HeadsetType

Headset
HeadsetType
Trigger
utils
warn
