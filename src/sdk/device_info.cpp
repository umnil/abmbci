#define UNICODE
#define NOMINMAX 1
#include <windows.h>
#include "sdk/sdk.hpp"
#include <string>

std::wstring device_info_get_device_name(_DEVICE_INFO const& di) {
    return std::wstring(di.chDeviceName);
}

void device_info_set_device_name(_DEVICE_INFO& di, std::wstring const& val) {
    std::copy(
        val.begin(),
        (val.size() < 256) ? val.end() : val.begin() + 255,
        di.chDeviceName
    );
}

std::wstring device_info_get_device_id(_DEVICE_INFO const& di) {
    return std::wstring(di.chDeviceID);
}

void device_info_set_device_id(_DEVICE_INFO& di, std::wstring const& val) {
    std::copy(
        val.begin(),
        (val.size() < MAX_PATH) ? val.end() : val.begin() + 255,
        di.chDeviceID
    );
}