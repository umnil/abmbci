#define UNICODE
#define NOMINMAX 1
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <windows.h>

#undef UNICODE
#include <iostream>

#include "headset/abmheadset.hpp"
#include "sdk/callbacks.hpp"
#include "sdk/channel_info.hpp"
#include "sdk/device_info.hpp"
#include "sdk/eeg_channel_info.hpp"
#include "sdk/electrodes_info.hpp"
#include "sdk/headset_type.hpp"
#include "sdk/logging.hpp"
#include "sdk/sdk.hpp"

namespace py = pybind11;

PYBIND11_MODULE(abmbciext, m) {
  m.doc() = "Advanced Brain Monitoring B-Alert Python SDK";

  m.def("get_sdk_dir", []() { return __ABMSDK__; });

  // =======================================================
  // CLASSES
  // =======================================================
  py::class_<_DEVICE_INFO>(m, "DeviceInfo")
      .def_property("device_name", &device_info_get_device_name,
                    &device_info_set_device_name)
      .def_readwrite("comm_port", &_DEVICE_INFO::nCommPort)
      .def_readwrite("n_ecg_pos", &_DEVICE_INFO::nECGPos)
      .def_readwrite("n_channels", &_DEVICE_INFO::nNumberOfChannel)
      .def_readwrite("esu_type", &_DEVICE_INFO::nESUType)
      .def_readwrite("timestamp_type", &_DEVICE_INFO::nTymestampType)
      .def_readwrite("device_handle", &_DEVICE_INFO::nDeviceHandle)
      .def_property("device_id", &device_info_get_device_id,
                    &device_info_set_device_id);

  py::class_<ELECTRODE>(m, "Electrode")
      .def_property("name",
                    STRING_GETTER_SETTER(ELECTRODE, chName, std::wstring,
                                         20))  // name of electrode
      .def_readwrite("has_good_impedance",
                     &ELECTRODE::bImpedance)  // if impedance is well (low)
      .def_readwrite("impedance_value",
                     &ELECTRODE::fImpedanceValue);  // value of impedance

  py::class_<CHANNEL_INFO>(m, "ChannelInfo")
      .def_property("chName",
                    STRING_GETTER_SETTER(CHANNEL_INFO, chName, std::wstring,
                                         20))  // name of electrode
      .def_readwrite(
          "bTechnicalInfo",
          &CHANNEL_INFO::bTechnicalInfo);  // if impedance is well (low)

  py::class_<_EEGCHANNELS_INFO>(m, "EEGChannelsInfo")
      .def_property("channel_names", &eeg_channel_info_get_channel_names,
                    &eeg_channel_info_set_channel_names)
      .def_property("used_channels", &eeg_channel_info_get_channels_used,
                    &eeg_channel_info_set_channels_used)
      .def_property("quality_channels", &eeg_channel_info_get_quality_channels,
                    &eeg_channel_info_set_quality_channels)
      .def_property("cleanable_channels",
                    &eeg_channel_info_get_cleanable_channels,
                    &eeg_channel_info_set_cleanable_channels)
      .def_property("eeg_channels", &eeg_channel_info_get_eeg_channels,
                    &eeg_channel_info_set_eeg_channels)
      .def_readwrite("flex", &_EEGCHANNELS_INFO::bIsFlex)
      .def_readwrite("channel_map", &_EEGCHANNELS_INFO::nChannelMap);

  py::class_<_ELECTRODES_INFO>(m, "ElectrodesInfo")
      .def_readwrite("num_electrodes", &_ELECTRODES_INFO::nNumElectrodes)
      .def_readwrite("stabilization", &_ELECTRODES_INFO::nStabilization)
      .def_readwrite("num_aggregation_samples",
                     &_ELECTRODES_INFO::nAgregationsSamples)
      .def_readwrite("current_type", &_ELECTRODES_INFO::nCurrentType)
      .def_property(
          "electrode_names",
          LIST_STRING_GETTER(_ELECTRODES_INFO, cElName, std::wstring),
          LIST_STRING_SETTER(
              _ELECTRODES_INFO, cElName, MAX_NUM_ELECTRODE,
              std::wstring))  //[in]the name of electrode max 20 characters
      .def_property(
          "command",
          LIST_GETTER(_ELECTRODES_INFO, nElCommand, MAX_NUM_ELECTRODE, int),
          LIST_SETTER(_ELECTRODES_INFO, nElCommand, MAX_NUM_ELECTRODE,
                      int))  // Impedance command to be sent for this electrode
                             // to be measured
      .def_property(
          "channels",
          LIST_GETTER(_ELECTRODES_INFO, nElChannel, MAX_NUM_ELECTRODE, int),
          LIST_SETTER(_ELECTRODES_INFO, nElChannel, MAX_NUM_ELECTRODE,
                      int))  // EEG channel to be used when measuring electrode
      .def_property("ref_electrodes",
                    LIST_GETTER(_ELECTRODES_INFO, nElReferentialElectrode,
                                MAX_NUM_ELECTRODE, int),
                    LIST_SETTER(_ELECTRODES_INFO, nElReferentialElectrode,
                                MAX_NUM_ELECTRODE,
                                int));  // Electrode to be used when
                                        // substracting ref el. (-1 for none)

  py::class_<_AUXDATA_INFO>(m, "AuxDataInfo")
      .def_readwrite("is_ired", &_AUXDATA_INFO::bIred)
      .def_readwrite("is_red", &_AUXDATA_INFO::bRed)
      .def_readwrite("has_tilt", &_AUXDATA_INFO::bTilt)
      .def_readwrite("ecg_index", &_AUXDATA_INFO::nEcgIndex)
      .def_readwrite("has_mic", &_AUXDATA_INFO::bMic)
      .def_readwrite("has_haptics", &_AUXDATA_INFO::bHaptic);

  py::class_<_HARDWARE_INFO>(m, "HardwareInfo")
      .def_readwrite("max_battery", &_HARDWARE_INFO::nBatteryMax)  // millivolts
      .def_readwrite("min_battery", &_HARDWARE_INFO::nBatteryMin)  // millivolts
      .def_readwrite("tilt_a", &_HARDWARE_INFO::nTiltLinearTransformA)
      .def_readwrite("tilt_b", &_HARDWARE_INFO::nTiltLinearTransformB);

  py::class_<_SESSIONTYPES_INFO>(m, "SessionTypesInfo")
      .def_readwrite("is_decon_supported",
                     &_SESSIONTYPES_INFO::bDecon)  // whether decontamination is
                                                   // supported or not
      .def_readwrite(
          "classification_supported",
          &_SESSIONTYPES_INFO::bBalert)  // whether b-alert classification is
                                         // supported or not
      .def_readwrite(
          "workload_supported",
          &_SESSIONTYPES_INFO::bWorkload);  // whether workload calculation is
                                            // supported or not

  py::class_<_CHANNELMAP_INFO>(m, "ChannelMapInfo")
      .def_readwrite("device_type", &_CHANNELMAP_INFO::nDeviceTypeCode)
      .def_readwrite("size", &_CHANNELMAP_INFO::nSize)
      .def_readwrite("eeg_channels", &_CHANNELMAP_INFO::stEEGChannels)
      .def_readwrite("electrodes", &_CHANNELMAP_INFO::stElectrodes)
      .def_readwrite("aux_data", &_CHANNELMAP_INFO::stAuxData)
      .def_readwrite("hardware_info", &_CHANNELMAP_INFO::stHardwareInfo)
      .def_readwrite("session_types", &_CHANNELMAP_INFO::stSessionTypes);

  py::class_<_STATUS_INFO>(m, "StatusInfo")
      .def_readwrite("battery_voltage", &_STATUS_INFO::BatteryVoltage)
      .def_readwrite("battery_percentage", &_STATUS_INFO::BatteryPercentage)
      .def_readwrite("timestamp", &_STATUS_INFO::Timestamp)
      .def_readwrite("total_missed_blocks", &_STATUS_INFO::TotalMissedBlocks)
      .def_readwrite("abmsdk_mode", &_STATUS_INFO::ABMSDK_Mode)
      .def_readwrite("last_error_code", &_STATUS_INFO::LastErrorCode)
      .def_readwrite("custom_mark_a", &_STATUS_INFO::CustomMarkA)
      .def_readwrite("custom_mark_b", &_STATUS_INFO::CustomMarkB)
      .def_readwrite("custom_mark_c", &_STATUS_INFO::CustomMarkC)
      .def_readwrite("custom_mark_d", &_STATUS_INFO::CustomMarkD)
      .def_readwrite("total_samples_received",
                     &_STATUS_INFO::nTotalSamplesReceived)
      .def_readwrite("online_imp_status", &_STATUS_INFO::OnLineImpStatus)
      .def_property("online_imp_values",
                    LIST_GETTER(_STATUS_INFO, OnLineImpValues, 24, int),
                    LIST_SETTER(_STATUS_INFO, OnLineImpValues, 24, int));

  py::enum_<HeadsetType>(m, "HeadsetType")
      .value("X24_QEEG", HeadsetType::X24_QEEG)
      .value("X24_STANDARD", HeadsetType::X24_STANDARD)
      .value("X10_STANDARD", HeadsetType::X10_STANDARD)
      .value("X24t_10_20", HeadsetType::X24t_10_20)
      .value("X10t_STANDARD", HeadsetType::X10t_STANDARD)
      .value("X24t_REDUCED", HeadsetType::X24t_REDUCED)
      .export_values();

  py::class_<ABMHeadset>(m, "Headset")
      .def(py::init<HeadsetType>(), py::arg("headset_type") = HeadsetType::X24_QEEG)
      .def("init", &ABMHeadset::init, py::arg("log_path") = "")
      .def("get_battery_percentage", &ABMHeadset::get_battery_percentage)
      .def("set_destination_file", &ABMHeadset::set_destination_file)
      .def("get_electrode_names", &ABMHeadset::get_electrode_names)
      .def("get_impedance_values", &ABMHeadset::get_impedance_values)
      .def("get_raw_data", &ABMHeadset::get_raw_npdata,
           py::arg("block") = false)
      .def("get_state", &ABMHeadset::get_state)
      .def("get_technical_data", &ABMHeadset::get_technical_data)
      .def("stop_acquisition", &ABMHeadset::stop_acquisition);

  // =======================================================
  // Functions
  // =======================================================

  m.def("get_log_path", &get_log_path);
  m.def("set_log_path", &set_log_path);
  m.def("ensure_log_path", &ensure_logging);

  m.def(
      "get_device_info_keep_connection",
      [](_DEVICE_INFO* device_info) {
        return GetDeviceInfoKeepConnection(device_info);
      },
      py::arg("device_info") = nullptr);

  m.def("close_current_connection", []() {
    std::cout << "Closing Current Connection" << std::endl;
    int retval = CloseCurrentConnection();
    std::cout << "Connection closed" << std::endl;
    return retval;
  });

  m.def(
      "set_config_path",
      [](std::wstring& path) {
        if (path.back() != L'\\') {
          path += L"\\";
        }
        return SetConfigPath(path.data());
      },
      py::arg("path") = __CONFIG__);

  m.def(
      "init_session_for_current_connection",
      [](int device_type, int session_type, int device_handle, int play_ebs) {
        return InitSessionForCurrentConnection(device_type, session_type,
                                               device_handle, play_ebs);
      },
      py::arg("device_type"), py::arg("session_type") = 0,
      py::arg("device_handle") = -1, py::arg("play_ebs") = 0);

  m.def("start_acquisition_for_current_connection",
        []() { return StartAcquisitionForCurrentConnection(); });
  m.def("stop_acquisition_keep_connection",
        []() { return StopAcquisitionKeepConnection(); });

  m.def(
      "get_raw_data",
      [](_DEVICE_INFO* di, int n_samples) {
        float* data = GetRawData(n_samples);
        int n_cols = di->nNumberOfChannel + 6;
        return py::array_t<float>(
            {n_samples, n_cols}, {sizeof(float) * n_cols, sizeof(float)}, data);
      },
      py::arg("di"), py::arg("n_samples") = 1);

  m.def("check_selected_impedances_for_current_connection",
        &check_selected_impedances_for_current_connection);
  m.def("register_callback_impedance_electrode_finished_a",
        &register_callback_impedance_electrode_finished_a);
  m.def("register_callback_device_detection_info",
        &register_callback_device_detection_info);
  m.def("register_callback_on_status_info", &register_callback_on_status_info);

  // =======================================================
  // Enumerations
  // =======================================================
}
