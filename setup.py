import os

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup


ext_modules = [
    Pybind11Extension(
        "abmbci",
        ["src/abmbci.cpp"],
        include_dirs=[f"{os.environ['ABMSDK']}/include"]
    )
]

setup(
    name="abmbci",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext}
)
