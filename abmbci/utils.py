import os
import pandas as pd  # type: ignore
from datetime import datetime


def impedance_isoconvert(path: str) -> pd.DataFrame:
    imp_data: pd.DataFrame = pd.read_csv(path)
    fn: str = os.path.basename(path)
    ses, date_str, time_str, fn_str, ext_str = fn.split(".")
    time_str = imp_data.iloc[0, 0]
    date = datetime.strptime(date_str, "%d%m%y").date()
    time = datetime.strptime(time_str, "%H:%M:%S:%f").time()
    tz = datetime.now().astimezone().tzinfo
    real_time: datetime = datetime.combine(date, time, tz)
    iso_str: str = real_time.isoformat()
    imp_data.iloc[:, 0] = [iso_str] * imp_data.shape[0]
    return imp_data
