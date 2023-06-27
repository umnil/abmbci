import numpy as np
import pandas as pd  # type: ignore
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List


class TestHeadset:
    def __init__(self) -> None:
        """This is a mock headset for testing data collection"""
        self.states: List[str] = [
            "DISCONNECTED",
            "IDLE",
            "INITIALIZED",
            "IMPEDANCE",
            "TECHNICAL",
            "ACQUISITION",
        ]
        self._state: str = self.states[1]
        self.sfreq: int = 256

    def get_battery_percentage(self) -> float:
        return 100.0

    def get_data_keys(self) -> List[str]:
        return [
            "Epoch",
            "Offset",
            "Hour",
            "Min",
            "Sec",
            "µSec",
        ] + self.get_electrode_names()

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

    def get_impedance_values(self, electrode_names: List[str]) -> Dict:
        """This will save fake impedance data"""
        now: datetime = datetime.now()
        ms: int = int(now.strftime("%f"))
        ms = int(np.round(ms / 1000))
        now_str: str = now.strftime("%H:%M:%S") + f":{ms}"
        electrode_names: List[str] = ["Ref"] + electrode_names
        system_time: List[str] = [now_str] * len(electrode_names)
        values: List[int] = [5] * len(electrode_names)
        statuses: List[str] = ["Good"] * len(electrode_names)
        dataframe: pd.DataFrame = pd.DataFrame(
            {
                "System Time": system_time,
                "Channel": electrode_names,
                "Value": values,
                "Status": statuses,
            }
        )
        impedance_fn: str = ".".join([self.file_prefix, "Impedance", "csv"])
        impedance_fp: Path = self.dest_path / impedance_fn
        dataframe.to_csv(impedance_fp, index=False)

        return {k: v for k, v in zip(electrode_names, values)}

    def get_technical_data(self) -> Dict:
        return {i.lower(): True for i in self.get_electrode_names()}

    def get_raw_data(self, block: bool = False) -> np.ndarray:
        if self._state != self.states[-1]:
            self._state = self.states[-1]
            self.start_time = datetime.now()
            self.last_sampled_time = datetime.now()

        data: np.ndarray = self.sample_test_data()
        if block:
            while data.shape[0] < 1:
                data = self.sample_test_data()
        return data

    def init(self) -> int:
        return 0

    def sample_test_data(self) -> np.ndarray:
        now: datetime = datetime.now()
        dt: timedelta = now - self.last_sampled_time
        elapsed_seconds: float = dt.total_seconds()
        n_samples: int = int(elapsed_seconds * self.sfreq)
        n_cols: int = len(self.get_electrode_names())
        n_rows: int = n_samples
        epochs: List[int] = [0] * n_samples
        offset: List[int] = [0] * n_samples
        T: timedelta = timedelta(seconds=1 / self.sfreq)
        times: List[datetime] = [
            self.last_sampled_time + (T * i) for i in range(1, n_samples + 1)
        ]
        hours: List[int] = [i.hour for i in times]
        minutes: List[int] = [i.minute for i in times]
        seconds: List[int] = [i.second for i in times]
        microseconds: List[int] = [i.microsecond for i in times]
        metadata: np.ndarray = np.c_[
            epochs, offset, hours, minutes, seconds, microseconds
        ]
        data: np.ndarray = np.random.rand(n_rows, n_cols)
        data = np.c_[metadata, data]
        self.last_sampled_time = (
            self.last_sampled_time if n_samples < 1 else times[-1]
            )
        return data

    def set_destination_file(self, dst: Path):
        self.dest_filepath: Path = dst
        self.dest_path: Path = dst.parent
        self.dest_path.mkdir(exist_ok=True, parents=True)
        self.dest_filename: str = dst.name
        components: List[str] = self.dest_filename.split(".")
        self.dest_prefix: str = ".".join(components[:-1])
        now: datetime = datetime.now()
        self.date_prefix: str = now.strftime("%d%m%y.%H%M%S")
        self.file_prefix: str = ".".join([self.dest_prefix, self.date_prefix])

    def stop_acquisition(self) -> None:
        self._state = self.states[1]
