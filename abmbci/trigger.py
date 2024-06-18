from _packet import OutPacket
from serial import Serial
from serial.tools.list_ports import comports
from serial.tools.list_ports_common import ListPortInfo
from typing import Optional


class Trigger:
    def __init__(self):
        self._port: Optional[Serial] = None
        self._port_name: Optional[str] = None
        self._hwid: str = "ACPI\\PNP0501\\1"

    @property
    def port(self) -> Serial:
        if self._port is not None:
            return self._port

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
            raise Exception("Failed to find trigger port")

        self._port_name = potential_port[0].device
        return self._port_name

    def send(self, data: str) -> None:
        packet: OutPacket = OutPacket(data)
        self.port.write(packet.encode())
