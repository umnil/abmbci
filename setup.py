import os

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup


ext_modules = [
    Pybind11Extension(
        "abmbci",
        ["src/abmbci.cpp"],
        include_dirs=[f"{os.environ['ABMSDK']}/include"],
        library_dirs=[f"{os.environ['ABMSDK']}/lib"],
        libraries=["ABM_Athena"]
    )
]

setup(
    name="abmbci",
    data_files=[
        (
            "lib\\site-packages\\", 
            [
                os.path.join(os.environ["ABMSDK"], "bin", x)
                for x in os.listdir(os.path.join(os.environ["ABMSDK"], "bin"))
            ]
        )
    ],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext}
)
