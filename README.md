# abmbci
The Advanced Brain Monitoring systems [EEG series
headsets](https://www.advancedbrainmonitoring.com/products/b-alert-x-series)
utilize libraries with and SDK. This repository ports functionality of these
libraries through C++ to python for use in brain-computer interface
experiments.

## Installation
Installation requires access to the ABM SDK libraries, and the environment
variable `AMBSDK` should point to the SDK path

```bash

# bash/sh
export ABMSDK=path/to/sdk

# powershell
$env:ABMSDK="path\to\sdk"
```

Unfortunately, these libraries only work in 32-bit DLLs for windows, meaning
these will only be compatible with windows software and in a 32-bit
environment. This is easily resolved using conda:

```bash

# NOTE: This is only necessary if attempting to actually get the device
# working on a windows machine. If only seeking to use the mock system
# do not set this and force 32BIT.
export CONDA_FORCE_32BIT=1
conda create -n environment_name

```

This will ensure that all install libraries are 32-bit including python

Then install with pip

```bash

pip install git+https://github.com/umnil/abmsdk

```

## Usage
Simply import the `Headset` interface to use

```python
from abmsdk import Headset

headset = Headset()

# sample data
data = headset.get_raw_data()
```

## Support
Please file a bug report or feature request in the issues tab.

## Contributing
Feel free to make pull request contributions. We encourage use of flake8
/ black styling

## Disclaimer
We are not affiliated with Advanced Brain Monitoring (ABM) Systems. ABMSDK
libraries are only made available through ABM and associated purchases
