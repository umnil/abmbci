#include <pybind11/pybind11.h>

int add (int i, int j) {
  return i + j;
}

PYBIND11_MODULE(abmbci, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Python SDK";
  m.def("add", &add, "Add two numbers");
}
