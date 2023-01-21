#define UNICODE
#include <windows.h>
#include <tchar.h>
#include "AbmSdkInclude.h"

#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace py = pybind11;

// Impedance
py::object selected_impedances_callback;

void __stdcall std_selected_impedances_callback(ELECTRODE* e, int& i) {
    if (_Py_IsFinalizing()) {
        std::cout << "Selected Impedance Callback: Finalizing" << std::endl;
        return;
    }
    py::gil_scoped_acquire gil;
    py::scoped_ostream_redirect output;
    selected_impedances_callback(e, i);
}

int check_selected_impedances_for_current_connection(py::object& callback, std::vector<std::wstring>& channels) {
    selected_impedances_callback = callback;
    std::vector<TCHAR*> p;
    for (std::wstring& c : channels) { p.push_back(c.data()); }
    return CheckSelectedImpedancesForCurrentConnection(std_selected_impedances_callback, p.data(), channels.size(), 0);
}

py::object impedance_electrode_finished_a_callback;

void __stdcall std_impedance_electrode_finished_a_callback(TCHAR* channel_name, float imp) {
    if (_Py_IsFinalizing()) {
        std::cout << "Electrode Impedance Callback: Finalizing!" << std::endl;
        return;
    }
    py::gil_scoped_acquire gil;
    py::scoped_ostream_redirect output;
    impedance_electrode_finished_a_callback(channel_name, imp);
}

int register_callback_impedance_electrode_finished_a(py::object& callback) {
    impedance_electrode_finished_a_callback = callback;
    return RegisterCallbackImpedanceElectrodeFinishedA(std_impedance_electrode_finished_a_callback);
}

// Device Detection Info
py::object device_detection_info_callback;

void __stdcall std_device_detection_info_callback(TCHAR* message, int i) {
    device_detection_info_callback(message, i);
}

int register_callback_device_detection_info(py::object& callback) {
    device_detection_info_callback = callback;
    return RegisterCallbackDeviceDetectionInfo(std_device_detection_info_callback);
}

// Status Info
py::object status_info_callback;

void __stdcall std_status_info_callback(_STATUS_INFO* status_info) {
    if (_Py_IsFinalizing()) {
        std::cout << "Status Info Callback: Finalizing!" << std::endl;
        return;
    }
    py::gil_scoped_acquire gil;
    py::scoped_ostream_redirect output;
    status_info_callback(status_info);
}

int register_callback_on_status_info(py::object& callback) {
    status_info_callback = callback;
    return RegisterCallbackOnStatusInfo(std_status_info_callback);
}