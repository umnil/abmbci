from abmbciext import OutPacket
from serial import Serial
from serial.tools.list_ports import ListPortInfo, comports
from typing import Optional


class Trigger:
    def __init__():
        self._pid: int = 0
        self._port: Optional[Serial] = None
        self._port_name: Optional[str] = None
        self._vid: int = 0

    @property
    def port(self) -> Serial:
        if self._port is not None:
            return self._port

        self._port = Serial(self.port_name)
        return self._port

    @property
    def port_name(self) -> str:
        if self._port_name is not None:
            return self._port_name

        potential_port: List[ListPortInfo] = [
            dev
            for dev in comports()
            if (dev.pid == self._pid) and (dev.vid == self._vid)
        ]

        if len(potential_port) < 1:
            raise Exception("Failed to find trigger port")

        self._port_name = potential_port[0].device_name
        return self._port_name

    def send(data: str) -> None:
        packet: OutPacket = OutPacket(data)
        self.port.write(packet.encode())
