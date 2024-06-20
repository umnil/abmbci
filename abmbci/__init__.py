from warnings import warn
from . import utils
from ._packet import InPacket, OutPacket  # type: ignore
from .trigger import Trigger

try:
    from abmbci._abmbci import Headset, HeadsetType  # type: ignore
except ModuleNotFoundError:
    warn("ABM software not available switching to mock system")
    from .mock_abm import Headset, HeadsetType

Headset
HeadsetType
InPacket
OutPacket
Trigger
utils
warn
