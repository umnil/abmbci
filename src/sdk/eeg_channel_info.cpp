#define UNICODE
#define NOMINMAX 1
#include <windows.h>
#include "sdk/sdk.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <string>
#include <vector>

namespace py = pybind11;

std::vector<std::string> eeg_channel_info_get_channel_names(_EEGCHANNELS_INFO const& eci) {
    std::vector<std::string> channel_names;
    for (auto name : eci.cChName) {
        channel_names.push_back(name);
    }
    return channel_names;
}

void eeg_channel_info_set_channel_names(_EEGCHANNELS_INFO& eci, std::vector<std::string> const& channel_names) {
    for (uint32_t i = 0; i < MAX_NUM_EEGCHANNELS; i++) {
        std::memset(eci.cChName[i], 0, MAX_LENGTH_CHANNEL_NAME);
        if (i < channel_names.size()) {
            auto end = (channel_names[i].size() < MAX_LENGTH_CHANNEL_NAME) ? channel_names[i].end() : channel_names[i].begin() + (MAX_LENGTH_CHANNEL_NAME - 1);
            std::copy(channel_names[i].begin(), end, eci.cChName[i]);
        }
    }
}

std::vector<bool> eeg_channel_info_get_channels_used(_EEGCHANNELS_INFO const& eci) {
    return std::vector<bool>(&eci.bChUsed[0], &eci.bChUsed[MAX_NUM_EEGCHANNELS]);
}

void eeg_channel_info_set_channels_used(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val) {
    std::memset(eci.bChUsed, 0, MAX_NUM_EEGCHANNELS);
    std::copy(
        val.begin(),
        (val.size() < MAX_NUM_EEGCHANNELS) ? val.end() : val.begin() + (MAX_NUM_EEGCHANNELS - 1),
        eci.bChUsed
    );
}

std::vector<bool> eeg_channel_info_get_quality_channels(_EEGCHANNELS_INFO const& eci) {
    return std::vector<bool>(&eci.bChUsedInQualityData[0], &eci.bChUsedInQualityData[MAX_NUM_EEGCHANNELS]);
}

void eeg_channel_info_set_quality_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val) {
    std::memset(eci.bChUsedInQualityData, 0, MAX_NUM_EEGCHANNELS);
    std::copy(
        val.begin(),
        (val.size() < MAX_NUM_EEGCHANNELS) ? val.end() : val.begin() + (MAX_NUM_EEGCHANNELS - 1),
        eci.bChUsedInQualityData
    );
}

std::vector<bool> eeg_channel_info_get_cleanable_channels(_EEGCHANNELS_INFO const& eci) {
    return std::vector<bool>(&eci.bChCanBeDecontaminated[0], &eci.bChCanBeDecontaminated[MAX_NUM_EEGCHANNELS]);
}

void eeg_channel_info_set_cleanable_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val) {
    std::memset(eci.bChCanBeDecontaminated, 0, MAX_NUM_EEGCHANNELS);
    std::copy(
        val.begin(),
        (val.size() < MAX_NUM_EEGCHANNELS) ? val.end() : val.begin() + (MAX_NUM_EEGCHANNELS - 1),
        eci.bChCanBeDecontaminated
    );
}

std::vector<bool> eeg_channel_info_get_eeg_channels(_EEGCHANNELS_INFO const& eci) {
    return std::vector<bool>(&eci.bIsChEEG[0], &eci.bIsChEEG[MAX_NUM_EEGCHANNELS]);
}

void eeg_channel_info_set_eeg_channels(_EEGCHANNELS_INFO& eci, std::vector<bool> const& val) {
    std::memset(eci.bIsChEEG, 0, MAX_NUM_EEGCHANNELS);
    std::copy(
        val.begin(),
        (val.size() < MAX_NUM_EEGCHANNELS) ? val.end() : val.begin() + (MAX_NUM_EEGCHANNELS - 1),
        eci.bIsChEEG
    );
}