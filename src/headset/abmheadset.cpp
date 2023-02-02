#ifdef __PYBIND11__
#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#endif  /* __PYBIND11__ */

#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include "headset/abmheadset.hpp"
#include "sdk/sdk.hpp"

#ifdef __PYBIND11__
namespace py = pybind11;
#endif  /* __PYBIND11__ */

std::function<void(std::wstring const&)> devcb;
std::function<void(_STATUS_INFO*)> statcb;
std::function<void(ELECTRODE*, int&)> impcb;
std::function<void(std::string const&, float const&)> impelcb;
std::function<void(CHANNEL_INFO* chInfo, int& n)> techcb;

void __stdcall devcb_trampoline(wchar_t* message, int i) {
    if (devcb) devcb(message);
}

void __stdcall statcb_trampoline(_STATUS_INFO* status_info) {
    if (statcb) statcb(status_info);
}

void __stdcall impcb_trampoline(ELECTRODE* pEl, int& i) {
    if (impcb) impcb(pEl, i);
}

void __stdcall impelcb_trampoline(TCHAR* ch, float i) {
    std::wstring wch = ch;
    std::string sch(wch.begin(), wch.end());
    if (impelcb) impelcb(std::string(sch), i);
}

void __stdcall techcb_trampoline(CHANNEL_INFO* ch, int& n) {
    if (techcb) techcb(ch, n);
}

ABMHeadset::~ABMHeadset(void) {
    if (!this->initialized_) return;
    if (this->connected_) {
        this->disconnect_();
    }
}

float ABMHeadset::get_battery_percentage(void) {
    std::lock_guard<std::mutex> lock(this->battery_mutex_);
    return this->battery_percentage_;
}

std::vector<std::string> ABMHeadset::get_data_keys(void) {
    std::vector<std::string> data_keys = {"Epoch", "Offset", "Hour", "Min", "Sec", "µSec"};
    for (std::string& c : this->electrode_names_) data_keys.push_back(c);
    return data_keys;
}

std::vector<std::string> ABMHeadset::get_electrode_names(void) {
#ifdef __PYBIND11__
    py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
    this->start_session_();
    if (this->electrode_names_.size() != this->num_channels_) {
        _CHANNELMAP_INFO channel_map;
        if (!GetChannelMapInfo(channel_map)) {
            this->print("Failed to get channel data");
            return this->electrode_names_;
        }
        for (int i = 0; i < this->num_channels_; i++) {
            if (!channel_map.stEEGChannels.bChUsed[i]) continue;
            this->electrode_names_.emplace_back(channel_map.stEEGChannels.cChName[i]);
        }
    }
    return this->electrode_names_;
}

std::map<std::string, float>const & ABMHeadset::get_impedance_values(std::vector<std::string> electrodes) {
#ifdef __PYBIND11__
    py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
    // Check electrodes requested
    if (electrodes.size() == 0) {
        std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
        return this->prev_impedance_;
    }
    // Check state
    {
        std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
        if (this->state_ != State::IMPEDANCE) {
            if (!this->state_ != State::IDLE) this->force_idle_();
            lock.unlock();
            this->print("Starting impedance check");
            if (this->start_impedance_(electrodes) != -1) {
                lock.lock();
                this->prev_impedance_cv_.wait(lock, [&]{return this->state_ == State::IDLE;});
            }
        }
    }
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    return this->prev_impedance_;
}

std::map<std::string, bool>const& ABMHeadset::get_technical_data(void) {
#ifdef __PYBIND11__
    py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
    {
        std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
        if (this->state_ != State::TECHNICAL) {
            if (!this->state_ != State::IDLE) this->force_idle_();
            lock.unlock();
            this->print("Starting technical check");
            if (this->start_technical_() != -1) {
                lock.lock();
                this->prev_monitoring_cv_.wait(lock, [&]{return this->state_ == State::IDLE;});
            }
            else {
                this->print("Failed to start technical check");
            }
        }
    }
    std::lock_guard<std::mutex> lock(this->prev_monitoring_mutex_);
    return this->prev_monitoring_;
}

std::pair<float*, int> ABMHeadset::get_raw_data(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::ACQUISITION) {
        if (this->state_ != State::IDLE) this->force_idle_();
        if (this->start_acquisition_() == -1) {
            this->print("Failed to start acquisition");
            return {NULL, 0};
        }
    }
    int n = 1;
    float* data = GetRawData(n);
    return {data, n};
}

std::map<std::string, std::vector<float>> ABMHeadset::get_raw_data_vector(void) {
    std::pair<float*, int> data = this->get_raw_data();
    std::map<std::string, std::vector<float>> retval;
    std::vector<std::string> data_keys = this->get_data_keys();
    int p = 0;
    for(int i = 0; i < data.second; i++) {
        for (std::string& s : data_keys) {
            if (retval.contains(s)) retval[s].push_back(data.first[p]);
            else retval[s] = std::vector<float>(1, data.first[p]);
            p++;
        }
    }
    return retval;
}

int ABMHeadset::init(std::filesystem::path log_path) {
#ifdef __PYBIND11__
    py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
    // Log Path
    if (!log_path.empty()) {
        set_log_path(log_path.wstring());
        ensure_logging();
    }
    
    // Device Connection Callback
    devcb = std::bind(&ABMHeadset::callback_device_info_, this, std::placeholders::_1);
    if (!RegisterCallbackDeviceDetectionInfo(&devcb_trampoline)) {
        this->print("Failed to register device detection callback");
        return 1;
    }

    // Device Status Callback
    statcb = std::bind(&ABMHeadset::callback_status_info_, this, std::placeholders::_1);
    if (!RegisterCallbackOnStatusInfo(&statcb_trampoline)) {
        this->print("Failed to register the status callback");
        return 2;
    }

    impcb = std::bind(&ABMHeadset::callback_impedance_finished_, this, std::placeholders::_1, std::placeholders::_2);
    impelcb = std::bind(&ABMHeadset::callback_electrode_impedance_, this, std::placeholders::_1, std::placeholders::_2);
    if (!RegisterCallbackImpedanceElectrodeFinishedA(impelcb_trampoline)) {
        this->print("Failed to register impedance elctrode callback");
        return 3;
    }

    // Technical monitoring callback
    techcb = std::bind(&ABMHeadset::callback_monitoring_, this, std::placeholders::_1, std::placeholders::_2);

    // Config path
    std::filesystem::path config_path("C:\\ABM\\B-Alert\\Config\\");
#ifndef __ABMSDK__
    char* ABMSDK = std::getenv("ABMSDK");
#else
    std::wstring wsdk(__ABMSDK__);
    std::string sdk(wsdk.begin(), wsdk.end());
    char* ABMSDK = sdk.data();
#endif
    if (ABMSDK != nullptr) {
        config_path = ABMSDK;
        config_path = config_path / ".." / "Config";
        if (config_path.filename() != "") config_path /= "";
    }
    
    this->print(std::string("Config Path set to: ") + config_path.string());
    int result = SetConfigPath(config_path.wstring().data());
    if (result != 1) {
        this->print("Failed to set config path");
        this->disconnect_();
        return 5;
    }

    // Connect to the device
    if (!this->connect_()) {
        this->print("Failed to connect to the device");
        return 6;
    }

    this->initialized_ = true;
    return 0;
}

bool ABMHeadset::set_destination_file(std::filesystem::path const& destination_file) {
    this->destination_file_ = destination_file;
    return SetDestinationFile(destination_file.wstring().data());
}

void ABMHeadset::callback_device_info_(std::wstring const& message) {
    this->print(std::wstring(L"Device Message: ") + message);
}

void ABMHeadset::callback_electrode_impedance_(std::string const& channel, float const& impedance) {
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    this->prev_impedance_[channel] = impedance;
}

void ABMHeadset::callback_impedance_finished_(ELECTRODE* pEl, int& i) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    this->print("Impedance is finished");
    if (this->state_ == State::IMPEDANCE) this->state_ = State::IDLE;
    {
        std::lock_guard<std::mutex> lock(this->connected_mutex_);

    }
    this->disconnect_();
    this->prev_impedance_cv_.notify_all();
}

void ABMHeadset::callback_monitoring_(CHANNEL_INFO* ch, int& n) {
    std::lock_guard<std::mutex> lock(this->prev_monitoring_mutex_);
    for (int i = 0; i < n; ch++, i++) {
        std::wstring wchname = ch->chName;
        std::string chname(wchname.begin(), wchname.end());
        this->prev_monitoring_[chname] = ch->bTechnicalInfo;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
        this->state_ = State::IDLE;
    }
    this->disconnect_();
    this->prev_monitoring_cv_.notify_all();
}

void ABMHeadset::callback_status_info_(_STATUS_INFO* status_info) {
    std::lock_guard<std::mutex> lock(this->battery_mutex_);
    this->battery_percentage_ = static_cast<float>(status_info->BatteryPercentage);
}

int ABMHeadset::connect_(void) {
    std::lock_guard<std::mutex> lock(this->connected_mutex_);
    if (this->connected_) return 2;
    _DEVICE_INFO* device_info = GetDeviceInfoKeepConnection(NULL);
    if (device_info == nullptr) {
        return 0;
    }
    if (this->device_name_.empty()) {
        std::wstring wdevicename = device_info->chDeviceName;
        this->device_name_.assign(wdevicename.begin(), wdevicename.end());
        this->num_channels_ = device_info->nNumberOfChannel;
    }
    this->connected_ = true;
    return 1;
}

int ABMHeadset::disconnect_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (!this->connected_) return 0;
    int ret = 0;
    switch (this->state_) {
        case State::ACQUISITION:
            ret = StopAcquisitionKeepConnection();
            break;
        case State::IMPEDANCE:
            ret = StopImpedance();
            break;
        case State::TECHNICAL:
            ret = StopTechnicalMonitoring();
            break;
    }
    std::lock_guard<std::mutex> conn_lock(this->connected_mutex_);
    CloseCurrentConnection();
    this->connected_ = false;
    this->print("Disconnected");
    return ret;
}

void ABMHeadset::force_idle_(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ == State::IDLE) return;
    this->print("Forcing Idle State");
    switch (this->state_) {
        case State::ACQUISITION:
            lock.unlock();
            this->print("Stopping Acquisition");
            this->stop_acquisition_();
            break;
        case State::TECHNICAL:
            lock.unlock();
            this->print("Stopping Technical monitoring");
            this->stop_technical_();
            break;
        case State::IMPEDANCE:
            lock.unlock();
            this->print("Stopping Impedance");
            this->stop_impedance_();
            break;
    }
    return;
}

int ABMHeadset::start_acquisition_(void) {
    this->print("Acquisition func");
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    this->print("Checking Idle");
    if (this->state_ != State::IDLE) return -1;
    this->print("Checking start session");
    if (this->start_session_() != INIT_SESSION_OK) return -1;
    // Getl electrode names
    if (this->electrode_names_.size() == 0) this->get_electrode_names();
    this->print("Checking start acquisition");
    if (StartAcquisitionForCurrentConnection() != ACQ_STARTED_OK) return -1;
    this->print("Started Acquisition!");
    this->state_ = State::ACQUISITION;
    return 0;
}

int ABMHeadset::start_impedance_(std::vector<std::string> const& electrodes) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::IDLE) return -1;
    if (this->start_session_() != 1) return -1;
    
    std::vector<std::wstring> welectrodes;
    for (std::string const& s : electrodes) { welectrodes.emplace_back(s.begin(), s.end()); }
    std::vector<TCHAR*> electrode_names;
    for (std::wstring& w : welectrodes) { electrode_names.push_back(w.data()); }
    if (CheckSelectedImpedancesForCurrentConnection(impcb_trampoline, electrode_names.data(), electrode_names.size(), 0) != IMP_STARTED_OK) {
        return -1;
    }
    this->state_ = State::IMPEDANCE;
    return 0;
}

int ABMHeadset::start_session_(int device, int session_type) {
    if (!this->connect_()) {
        this->print("Failed to connect");
        return -1;
    }
    int retval = InitSessionForCurrentConnection(device, session_type, -1, false);
    return retval;
}

int ABMHeadset::start_technical_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::IDLE) return -1;
    if (this->start_session_(1, 1) != 1) return -1;
    if (TechnicalMonitoring(&techcb_trampoline, 15, NULL) != TM_STARTED_OK) return -1;
    this->state_ = State::TECHNICAL;
    return 0;
}

void ABMHeadset::stop_acquisition_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::ACQUISITION) return;
    StopAcquisitionKeepConnection();
    this->state_ = State::IDLE;
}

void ABMHeadset::stop_impedance_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::IMPEDANCE) return;
    StopImpedance();
    this->state_ = State::IDLE;
    this->prev_impedance_cv_.notify_all();
}

void ABMHeadset::stop_technical_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::TECHNICAL) return;
    this->state_ = State::IDLE;
}