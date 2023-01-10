#include "AbmSdkInclude.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(abmbci, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Python SDK";
  py::class_<_DEVICE_INFO>(m, "DeviceInfo")
    .def_readwrite("device_name", &_DEVICE_INFO::chDeviceName)
}
