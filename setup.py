import os
import sys
from skbuild import setup
from setuptools import find_packages
from pathlib import Path

sdk_dir = Path(os.environ["ABMSDK"])
system_dir = sdk_dir.parent
config_dir = system_dir / "Config"
sdk_bin_dir = sdk_dir / "bin"
sdk_lib_dir = sdk_dir / "lib"
sdk_inc_dir = sdk_dir / "include"

if sys.platform == "win32":
    # only build on win32
    data_files = [
        (
            "lib\\site-packages\\abmbci",
            [
                os.path.relpath(os.path.join(str(sdk_bin_dir), x))
                for x in os.listdir(sdk_bin_dir)
            ],
        )
    ]
else:
    data_files = []

setup(
    name="abmbci",
    packages=["abmbci"],
    data_files=data_files,
)
