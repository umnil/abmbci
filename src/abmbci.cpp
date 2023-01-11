#define UNICODE
#include <windows.h>
#include "AbmSdkInclude.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(abmbci, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Python SDK";
  py::class_<_DEVICE_INFO>(m, "DeviceInfo")
    .def_property("device_name", [](_DEVICE_INFO const& di){
      return std::wstring(di.chDeviceName);
    }, [](_DEVICE_INFO& di, std::wstring const& val){
      std::copy(
        val.begin(),
        (val.size() < 256) ? val.end() : val.begin() + 255,
        di.chDeviceName
      );
    })
    .def_readwrite("comm_port", &_DEVICE_INFO::nCommPort)
    .def_readwrite("n_ecg_pos", &_DEVICE_INFO::nECGPos)
    .def_readwrite("n_channels", &_DEVICE_INFO::nNumberOfChannel)
    .def_readwrite("esu_type", &_DEVICE_INFO::nESUType)
    .def_readwrite("timestamp_type", &_DEVICE_INFO::nTymestampType)
    .def_readwrite("device_handle", &_DEVICE_INFO::nDeviceHandle)
    .def_property("device_id", [](_DEVICE_INFO const& di){
      return std::wstring(di.chDeviceID);
    }, [](_DEVICE_INFO& di, std::wstring const& val){
      std::copy(
        val.begin(),
        (val.size() < MAX_PATH) ? val.end() : val.begin() + 255,
        di.chDeviceID
      );
    });

  m.def(
    "get_device_info_keep_connection",
    [](_DEVICE_INFO* device_info){ return GetDeviceInfoKeepConnection(device_info); },
    py::arg("device_info") = nullptr
  );

  m.def("set_config_path", [](std::wstring& path){ SetConfigPath(path.data()); });

  m.def(
    "init_session",
    [](int device_type, int session_type, int device_handle, int play_ebs ){
      return InitSession(device_type, session_type, device_handle, play_ebs);
    },
    py::arg("device_type"),
    py::arg("session_type"),
    py::arg("device_handle") = 0,
    py::arg("play_ebs") = 0
  );
}
