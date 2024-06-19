#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <iostream>


namespace py = pybind11;

PYBIND11_MODULE(_abmbci, m) {
  m.doc() = "STUB Advanced Brain Monitoring B-Alert Python SDK";

  m.def("get_sdk_dir", [](){ return __ABMSDK__; });

  //py::class_<ABMHeadset>(m, "Headset")
    //.def(py::init<>())
    //.def("init", &ABMHeadset::init, py::arg("log_path") = "")
    //.def("get_battery_percentage", &ABMHeadset::get_battery_percentage)
    //.def("set_destination_file", &ABMHeadset::set_destination_file)
    //.def("get_electrode_names", &ABMHeadset::get_electrode_names)
    //.def("get_impedance_values", &ABMHeadset::get_impedance_values)
    //.def("get_raw_data", &ABMHeadset::get_raw_npdata, py::arg("block") = false)
    //.def("get_state", &ABMHeadset::get_state)
    //.def("get_technical_data", &ABMHeadset::get_technical_data)
    //.def("stop_acquisition", &ABMHeadset::stop_acquisition);
}
