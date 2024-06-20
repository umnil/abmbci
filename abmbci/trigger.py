import warnings
from datetime import datetime, timedelta
from queue import Queue
from serial import Serial  # type: ignore
from serial.tools.list_ports import comports  # type: ignore
from serial.tools.list_ports_common import ListPortInfo  # type: ignore
from typing import Any, List, Optional, TYPE_CHECKING, cast
from ._packet import InPacket, OutPacket  # type: ignore

if TYPE_CHECKING:
    from abmbci import Headset


class Trigger:
    def __init__(self, headset: Optional["Headset"] = None):
        """A trigger class for sending triggers to the embedded synchronization
        unit for ABM. If no port is available a mock trigger can be used by
        provided the headset to pass the data virtually to the headset
        """
        self._port: Optional[Serial] = None
        self._port_name: Optional[str] = None
        self._hwid: str = "ACPI\\PNP0501\\1"
        self._headset: Optional["Headset"] = headset

    @property
    def port(self) -> Serial:
        if self._port is not None:
            return self._port

        if self.port_name == "":
            self._port = cast("Headset", self._headset).trigger_file
        else:
            self._port = Serial(self.port_name, 57600)
        return self._port

    @property
    def port_name(self) -> str:
        if self._port_name is not None:
            return self._port_name

        potential_port: List[ListPortInfo] = [
            dev for dev in comports() if dev.hwid == self._hwid
        ]

        if len(potential_port) < 1:
            warnings.warn("Failed to find trigger port. Reverting to Mock Trigger")
            if self._headset is None:
                raise Exception("No headset available. Cannot create mock trigger")

            self._headset.trigger_file = Queue()
            cast(Any, self._headset.trigger_file).write = self.store
            return ""

        self._port_name = potential_port[0].device
        return self._port_name

    def send(self, data: str) -> None:
        packet: OutPacket = OutPacket(data)
        self.port.write(packet.encode())

    def store(self, data: bytes) -> None:
        o_packet: OutPacket = OutPacket.from_bytes(data)
        timestamp: timedelta = datetime.now() - cast("Headset", self._headset).init_time
        packet: InPacket = InPacket(timestamp, o_packet.data)
        cast("Headset", self._headset).trigger_file.put_nowait(bytes(packet.data))
