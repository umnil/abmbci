import mne  # type: ignore
import numpy as np
import pandas as pd  # type: ignore
from datetime import datetime, timedelta
from enum import Enum
from pathlib import Path
from typing import Dict, List, Optional


class HeadsetType(Enum):
    X24_QEEG = 0
    X24_STANDARD = 1
    X10_STANDARD = 2
    X24t_10_20 = 6
    X10t_STANDARD = 7
    X24t_REDUCED = 8


class HeadsetState(Enum):
    DISCONNECTED = "DISCONNECTED"
    IDLE = "IDLE"
    INITIALIZED = "INITIALIZED"
    IMPEDANCE = "IMPEDANCE"
    TECHNICAL = "TECHNICAL"
    ACQUISITION = "ACQUISITION"


HEADSET_CHANNELS = {
    HeadsetType.X24_QEEG: [
        "Fp1",
        "F7",
        "F8",
        "T4",
        "T6",
        "T5",
        "T3",
        "Fp2",
        "O1",
        "P3",
        "Pz",
        "F3",
        "Fz",
        "F4",
        "C4",
        "P4",
        "POz",
        "C3",
        "Cz",
        "O2",
        "ECG",
        "AUX1",
        "AUX2",
        "AUX3",
    ],
    HeadsetType.X24_STANDARD: [
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
    ],
}


class TestHeadset:
    def __init__(
        self, headset_type: HeadsetType, sample_data: List[mne.io.Raw] = []
    ) -> None:
        r"""This is a mock headset for testing data collection

        Notes on notation:
        $f_s$: `sfreq` : sampling frequency
        $n_0$: `start_sample` start sample number (typically 0)
        $n$: `prev_sample`: last sampled index
        $t_0$: `start_time`: Start time
        $t_c$: `cur_time`: Current Time Now Live
        $n_c = [\frac{t_c}{f_s}]$: `cur_sample`: current sample index live (i.e., the total number of samples collected)
        $t = n * f_s$: `prev_time`: Last sampled time
        $\Delta t = t_c - t$: `elapsed_prev_time`: elapsed time since last sample
        $\Delta t_0 = t_c - t_0$: `elapsed_start_time` : total elapsed time since start
        $\Delta n$: `elapsed_prev_sample`: number of samples lapsed since last sample
        $\Delta n_0$: `elapsed_start_sample`: number of samples lapsed since beginning
        $\vec{R}$: `raw_runs`: a vector of `mne.io.raw` objects for each run in the sample data set
        $\vec{L^{(t)}}$: `time_runs` : a vector of time lengths, where each $i$-th ($L^{(t)}_i$) element is the length in time of $R_i$
        $\vec{L^{(n)}}$: `sample_runs` : a vector of sample lengths, where each $i$-th element is the length in samples of $R_i$
        $\mathbf{D} \in \mathbb{R}^{f \times c \times n}$: `data_runs`: a matrix of data arrays
        $\Psi(i) = \sum_{k=0}^{i} L^{(n)}_i$: The cumulative summation of $\vec{L^{(n)}}$
        $H(x)$: Heaviside equation
        $r_c = \underset{i}{argmax} \left( H(\psi(i) - n_c)  \right)$: `cur_run`

        Parameters
        ----------
        headset_type : HeadsetType
            The type of headset to mock. Determines available channels
        sample_data : List[mne.io.Raw]
            By default, data sampled from the mock headset is random floating
            point values between 0 and 1 provided by numpy's random number
            generator. However, if sampling real EEG data is preferrable to
            simulate, this allows for a list of mne Raw objects to be provided
            and the headset with sample data from these objects
        """
        self._data_runs: List[np.ndarray] = []
        self._headset_type: HeadsetType = headset_type
        self._state: HeadsetState = HeadsetState.IDLE
        self.cur_time: Optional[datetime] = None
        self.prev_sample: int = 0
        self.raw_runs: List[mne.io.Raw] = sample_data
        self.start_sample: int = 0
        self.start_time: Optional[datetime] = None

    @property
    def cur_run(self) -> np.int32:
        return np.argmax(np.heaviside(np.cumsum(self.sample_runs) - self.cur_sample, 1))

    @property
    def cur_run_data(self) -> np.ndarray:
        return self.data_runs[self.cur_run]

    @property
    def cur_sample(self) -> int:
        if self.cur_time is None or self.start_time is None:
            raise Exception("Headset is not currently sampling data")
        return self.t2n(self.cur_time - self.start_time)

    @property
    def data_runs(self) -> List:
        if len(self._data_runs) < 1:
            self._data_runs = [i.get_data() for i in self.raw_runs]

        return self._data_runs

    @property
    def elapsed_prev_sample(self) -> int:
        return self.cur_sample - self.prev_sample

    @property
    def elapsed_prev_time(self) -> timedelta:
        if self.cur_time is None or self.prev_time is None:
            raise Exception("Headset is not currently sampling data")
        return self.cur_time - self.prev_time

    @property
    def elapsed_start_sample(self) -> int:
        return self.cur_sample - self.start_sample

    @property
    def elapsed_start_time(self) -> timedelta:
        if self.cur_time is None or self.start_time is None:
            raise Exception("Headset is not currently sampling data")
        return self.cur_time - self.start_time

    def get_battery_percentage(self) -> float:
        """Return a fake battery percentage

        Returns
        -------
        float
            A floating point value representing the battery life
        """
        return 100.0

    def get_data_keys(self) -> List[str]:
        """The first six values in the raw data array correspond to metadata of
        6 values. This returns strings that describe these six fields

        Returns
        -------
        List[str]
            A list of strings, where each string is a descriptive name
        """
        return [
            "Epoch",
            "Offset",
            "Hour",
            "Min",
            "Sec",
            "µSec",
        ] + self.get_electrode_names()

    def get_electrode_names(self) -> List[str]:
        """Get a list of strings where each string is a name for the electrode.
        The order of the electrodes in this list matches to order of the data
        in `get_raw_data`

        Returns
        -------
        List[str]
            A List of strings where each string is the name of the electrode
            used on the headset
        """
        return HEADSET_CHANNELS[self._headset_type]

    def get_impedance_values(self, electrode_names: List[str]) -> Dict:
        """This will save fake impedance data

        Parameters
        ----------
        electrode_names : List[str]
            A list of electrode names to check impedance on

        Returns
        -------
        Dict
            A dictionary of electrode names and their impedance values
        """
        now: datetime = datetime.now()
        ms: int = int(now.strftime("%f"))
        ms = int(np.round(ms / 1000))
        now_str: str = now.strftime("%H:%M:%S") + f":{ms}"
        electrode_names = ["Ref"] + electrode_names
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

    def get_raw_data(self, block: bool = False) -> np.ndarray:
        """Gather and emit sample raw data

        Parameters
        ----------
        block : bool
            If True, do not return until at least one sample is available for
            return

        Returns
        -------
        np.ndarray
            An array of raw data. May be empty if block is False and this
            function is called too quickly.
        """
        if self._state != HeadsetState.ACQUISITION:
            self.start_acquisition()

        data: np.ndarray = self.sample_data()
        if block:
            while data.shape[0] < 1:
                data = self.sample_data()
        return data

    def get_technical_data(self) -> Dict:
        """Obtain technical data on the electrodes

        Returns
        -------
        Dict
            A dictionary where the keys are electrode names and the values are
            boolean values wherein True indicates that the channel is in good
            quality.
        """
        return {i.lower(): True for i in self.get_electrode_names()}

    def init(self) -> int:
        """Dummy function for initialization"""
        return 0

    def n2t(self, n: int) -> timedelta:
        return n * self.period

    @property
    def period(self) -> timedelta:
        return timedelta(seconds=1 / self.sfreq)

    @property
    def prev_time(self) -> datetime:
        if self.start_time is None:
            raise Exception("Headset is not sampling")
        return self.start_time + (self.n2t(self.prev_sample))

    @property
    def sample_runs(self) -> List[int]:
        return [i.n_times for i in self.raw_runs]

    def sample_main_data(self, ni: int, nf: int) -> np.ndarray:
        if len(self.raw_runs) > 0:
            return self.cur_run_data[:, ni:nf]
        else:
            n_rows: int = len(self.get_electrode_names())
            return np.random.rand(n_rows, nf - ni)

    def sample_metadata(self, n: int) -> np.ndarray:
        if self.prev_time is None:
            raise Exception("Headset is not sampling")

        times: List[datetime] = [
            self.prev_time + (self.period * i) for i in range(0, n)
        ]
        return np.c_[
            [0] * n,  # epochs
            [0] * n,  # offsets
            [i.hour for i in times],  # hours
            [i.minute for i in times],  # minutes
            [i.second for i in times],  # second
            [i.microsecond for i in times],  # microseconds
        ]

    def sample_data(self) -> np.ndarray:
        self.cur_time = datetime.now()
        metadata: np.ndarray = self.sample_metadata(self.elapsed_prev_sample)
        main_data: np.ndarray = self.sample_main_data(self.prev_sample, self.cur_sample)
        data = np.c_[metadata, main_data.T]
        self.prev_sample = self.cur_sample
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

    @property
    def sfreq(self) -> int:
        return 256 if len(self.raw_runs) == 0 else self.raw_runs[0].info["sfreq"]

    def start_acquisition(self) -> None:
        self._state = HeadsetState.ACQUISITION
        self.start_time = datetime.now()
        self.start_sample = 0
        self.prev_sample = 0

    def stop_acquisition(self) -> None:
        self._state = HeadsetState.IDLE
        self.start_time = None
        self._data_runs = []

    @property
    def time_runs(self) -> List[timedelta]:
        return [timedelta(seconds=(n / self.sfreq)) for n in self.sample_runs]

    def t2n(self, t: timedelta) -> int:
        return int(np.round(t.total_seconds() * self.sfreq))
