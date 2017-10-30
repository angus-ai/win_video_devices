# Windows python video devices retriever

Retrieve the index and name of video devices connected to your Windows computer in Python

## Prerequisites

* Visual Studio 2015 (required for all Python versions)
* CMake >= 3.1

## Installation

Just clone this repository and pip install. Note the `--recursive` option which is
needed for the pybind11 submodule:

```bash
git clone --recursive https://github.com/angus-ai/win_video_devices.git
pip install .
```

With the `setup.py` file included in this project, the `pip install` command will
invoke CMake and build the pybind11 module as specified in `CMakeLists.txt`.

## Testing

You can launch a python interpreter and run:

```python
import win_devices
dev = win_devices.get_devices()
print(dev)
```
