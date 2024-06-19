#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>
#include "packet.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_packet, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Packet SDK";

  // =======================================================
  // CLASSES
  // =======================================================
  py::class_<OutPacket>(m, "OutPacket")
      .def(py::init<std::string const &>()) // Bind the constructor
      .def("encode",
           [](OutPacket &self) {
             int size;
             char *encoded = self.encode(&size);
             py::bytes result(encoded, size);
             delete[] encoded; // Clean up the allocated memory
             return result;
           })                                   // Bind the encode method
      .def_readwrite("data", &OutPacket::data); // Bind the data member

  py::class_<InPacket>(m, "InPacket")
      .def(py::init<char *, int>())
      .def_readonly("counter", &InPacket::counter)
      .def_readonly("data", &InPacket::data)
      .def_readonly("userdata", &InPacket::userdata)
      .def_readonly("timestamp", &InPacket::timestamp);
}
