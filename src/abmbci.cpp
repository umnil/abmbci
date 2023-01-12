#define UNICODE
#define NOMINMAX 1
#include <windows.h>
#include "AbmSdkInclude.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <iostream>

namespace py = pybind11;

PYBIND11_MODULE(abmbciext, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Python SDK";

  m.def("get_sdk_dir", [](){ return ABMSDK; });

  // =======================================================
  // CLASSES
  // =======================================================
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

  py::class_<_EEGCHANNELS_INFO>(m, "EEGChannelsInfo")
    .def_property("channel_names", [](_EEGCHANNELS_INFO const& eci) {
      std::vector<std::string> channel_names;
      for (auto name : eci.cChName) {
        channel_names.push_back(name);
      }
      return channel_names;
    }, [](_EEGCHANNELS_INFO& eci, std::vector<std::string> const& channel_names) {
      for (uint32_t i = 0; i < MAX_NUM_EEGCHANNELS; i++) {
        std::memset(eci.cChName[i], 0, MAX_LENGTH_CHANNEL_NAME);
        if (i < channel_names.size()) {
          auto end = (channel_names[i].size() < MAX_LENGTH_CHANNEL_NAME) ? channel_names[i].end() : channel_names[i].begin() + (MAX_LENGTH_CHANNEL_NAME - 1);
          std::copy(channel_names[i].begin(), end, eci.cChName[i]);
        }
      }
    })
    .def_property("used_channels", [](_EEGCHANNELS_INFO const& eci) {
      return py::array_t<bool>(
        { MAX_NUM_EEGCHANNELS },
        eci.bChUsed
      );
    }, [](_EEGCHANNELS_INFO& eci, py::array_t<bool> const& list){
      std:memset(eci.bChUsed, 0, MAX_NUM_EEGCHANNELS);
      std::copy(
        list.begin(),
        (list.size() < MAX_NUM_EEGCHANNELS) ? list.end() : list.begin() + (MAX_NUM_EEGCHANNELS - 1),
        eci.bChUsed
      );
    });

  // =======================================================
  // Functions
  // =======================================================

  m.def(
    "get_device_info_keep_connection",
    [](_DEVICE_INFO* device_info){ return GetDeviceInfoKeepConnection(device_info); },
    py::arg("device_info") = nullptr
  );

  m.def("close_current_connection", [](){ return CloseCurrentConnection(); });

  m.def(
    "set_config_path",
    [](std::wstring& path){ 
      if (path.back() != L'\\') {
        path += L"\\";
      }
      return SetConfigPath(path.data()); 
    },
    py::arg("path") = CONFIG
  );

  m.def(
    "init_session_for_current_connection",
    [](int device_type, int session_type, int device_handle, int play_ebs ){
      return InitSessionForCurrentConnection(device_type, session_type, device_handle, play_ebs);
    },
    py::arg("device_type"),
    py::arg("session_type") = 0,
    py::arg("device_handle") = -1,
    py::arg("play_ebs") = 0
  );

  m.def("start_acquisition_for_current_connection", [](){ return StartAcquisitionForCurrentConnection(); });
  m.def("stop_acquisition_keep_connection", [](){ return StopAcquisitionKeepConnection(); });

  m.def(
    "get_raw_data",
    [](_DEVICE_INFO* di, int n_samples){
      float* data = GetRawData(n_samples);
      int n_cols = di->nNumberOfChannel + 6;
      return py::array_t<float>(
        { n_samples, n_cols },
        { sizeof(float) * n_cols, sizeof(float) },
        data
      );
    },
    py::arg("di"),
    py::arg("n_samples") = 1
  );

}
