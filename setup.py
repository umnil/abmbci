import os
import sys

from pathlib import Path
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, find_packages  # type: ignore
from typing import List

sdk_dir = Path(os.environ["ABMSDK"])
system_dir = sdk_dir.parent
config_dir = system_dir / "Config"
sdk_bin_dir = sdk_dir / "bin"
sdk_lib_dir = sdk_dir / "lib"
sdk_inc_dir = sdk_dir / "include"

if sys.platform == "win32":
    # only build on win32
    ext_modules = [
        Pybind11Extension(
            "abmbciext",
            [
                "src/headset/abmheadset.cpp",
                "src/headset/packet.cpp",
                "src/sdk/callbacks.cpp",
                "src/sdk/device_info.cpp",
                "src/sdk/eeg_channel_info.cpp",
                "src/sdk/logging.cpp",
                "src/abmbci.cpp",
            ],
            include_dirs=[str(sdk_inc_dir), "include"],
            define_macros=[
                ("__PYBIND11__", "1"),
                ("__ABMSDK__", 'L"' + str(sdk_dir).replace("\\", "\\\\") + '"'),
                ("__CONFIG__", 'L"' + str(config_dir).replace("\\", "\\\\") + '"'),
            ],
            extra_compile_args=["/DWIN32", "/D_WINDOWS", "/DEBUG"],
            extra_linker_args=["/DEBUG"],
            library_dirs=[str(sdk_lib_dir)],
            libraries=["ABM_Athena"],
        )
    ]
    data_files = [
        (
            "lib\\site-packages\\",
            [
                os.path.relpath(os.path.join(str(sdk_bin_dir), x))
                for x in os.listdir(sdk_bin_dir)
            ],
        )
    ]
else:
    ext_modules: List = []
    data_files: List = []

setup(
    name="abmbci",
    packages=find_packages(),
    data_files=data_files,
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
