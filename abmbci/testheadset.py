from datetime import Datetime
from pathlib import Path
from typing import List


class TestHeadset:
    def __init__(self):
        """This is a mock headset for testing data collection"""

    def get_battery_percentage(self) -> float:
        return 100.0

    def get_electrode_names(self) -> List[str]:
        return [
            "F3",
            "F1",
            "Fz",
            "F2",
            "F4",
            "C3",
            "C1",
            "Cz",
            "C2",
            "C4",
            "CPz",
            "P3",
            "P1",
            "Pz",
            "P2",
            "P4",
            "POz",
            "O1",
            "Oz",
            "O2",
            "ECG",
            "AUX1",
            "AUX2",
            "AUX3",
        ]

    def get_impedance_values(self) -> Dict:
        """This will save fake impedance data"""

        return {
            k: 5 
            for k in self.get_electrode_names()
        }

    def init(self) -> int:
        return 0

    def set_destination_file(self, dst: Path):
        self.dest_filepath: Path = dst
        self.dest_path: Path = dst.parent
