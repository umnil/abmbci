#ifndef INCLUDE_CALLBACKS_HPP_
#define INCLUDE_CALLBACKS_HPP_
#include "AbmSdkInclude.h"

#include <pybind11/functional.h>

#include <functional>
#include <string>

namespace py = pybind11;

// Impedance
extern py::object selected_impedances_callback;
void __stdcall std_selected_impedances_callback(ELECTRODE* e, int& i);
int check_selected_impedances_for_current_connection(py::object& callback, std::vector<std::wstring>& channels);

extern py::object impedance_electrode_finished_a_callback;
void __stdcall std_impedance_electrode_finished_a_callback(TCHAR* channel_name, float imp);
int register_callback_impedance_electrode_finished_a(py::object& callback);

// Device Detection Info
extern py::object device_detection_info_callback;
void __stdcall std_device_detection_info_callback(TCHAR* message, int i);
int register_callback_device_detection_info(py::object& callback);

// Status Info
extern py::object status_info_callback;
void __stdcall std_status_info_callback(_STATUS_INFO* status_info);
int register_callback_on_status_info(py::object& callback);
#endif  /* INCLUDE_CALLBACKS_HPP_ */