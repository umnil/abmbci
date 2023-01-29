import os

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup, find_packages

sdk_dir = os.environ["ABMSDK"]
system_dir = os.path.dirname(sdk_dir)
config_dir = os.path.join(system_dir, "Config")
sdk_bin_dir = os.path.join(sdk_dir, "bin")
sdk_lib_dir = os.path.join(sdk_dir, "lib")
sdk_inc_dir = os.path.join(sdk_dir, "include")

ext_modules = [
    Pybind11Extension(
        "abmbciext",
        [
            "src/sdk/callbacks.cpp",
            "src/sdk/device_info.cpp",
            "src/sdk/eeg_channel_info.cpp",
            "src/sdk/logging.cpp",
            "src/abmbci.cpp"
        ],
        include_dirs=[sdk_inc_dir, "include"],
        define_macros=[
            ("ABMSDK", 'L"' + sdk_dir.replace("\\", "\\\\") + '"'),
            ("CONFIG", 'L"' + config_dir.replace("\\", "\\\\") + '"')
        ],
        extra_compile_args=["/DWIN32", "/D_WINDOWS", "/DEBUG"],
        extra_linker_args=["/DEBUG"],
        library_dirs=[sdk_lib_dir],
        libraries=["ABM_Athena"]
    )
]

setup(
    name="abmbci",
    version="0.1.0",
    packages=find_packages(),
    setup_requires=[
        "pybind11"
    ],
    data_files=[
        (
            "lib\\site-packages\\", 
            [
                os.path.join(sdk_bin_dir , x)
                for x in os.listdir(sdk_bin_dir)
            ]
        )
    ],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext}
)
