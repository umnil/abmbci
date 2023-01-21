#ifndef INCLUDE_DEVICE_INFO_HPP_
#define INCLUDE_DEVICE_INFO_HPP_
#include <string>
std::wstring device_info_get_device_name(_DEVICE_INFO const&);
void device_info_set_device_name(_DEVICE_INFO& di, std::wstring const& val);
std::wstring device_info_get_device_id(_DEVICE_INFO const& di);
void device_info_set_device_id(_DEVICE_INFO& di, std::wstring const& val);
#endif  /* INCLUDE_DEVICE_INFO_HPP_ */