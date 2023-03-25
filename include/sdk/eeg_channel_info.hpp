#ifndef INCLUDE_EEG_CHANNEL_INFO_HPP_
#define INCLUDE_EEG_CHANNEL_INFO_HPP_
#define UNICODE
#define NOMINMAX 1
#include <windows.h>
#include "sdk/sdk.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <string>
#include <vector>

namespace py = pybind11;

std::vector<std::string> eeg_channel_info_get_channel_names(_EEGCHANNELS_INFO const& eci);
void eeg_channel_info_set_channel_names(_EEGCHANNELS_INFO& eci, std::vector<std::string> const& channel_names);
std::vector<bool> eeg_channel_info_get_channels_used(_EEGCHANNELS_INFO const& eci);
void eeg_channel_info_set_channels_used(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val);
std::vector<bool> eeg_channel_info_get_quality_channels(_EEGCHANNELS_INFO const& eci);
void eeg_channel_info_set_quality_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val);
std::vector<bool> eeg_channel_info_get_cleanable_channels(_EEGCHANNELS_INFO const& eci);
void eeg_channel_info_set_cleanable_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val);
std::vector<bool> eeg_channel_info_get_eeg_channels(_EEGCHANNELS_INFO const& eci);
void eeg_channel_info_set_eeg_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val);
#endif  /* INCLUDE_EEG_CHANNEL_INFO_HPP_ */