#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include "headset/abmheadset.hpp"
#include "sdk/sdk.hpp"

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

std::map<std::string, float>const & ABMHeadset::get_impedance_values(std::vector<std::string> electrodes) {
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
            if (this->start_impedance_(electrodes) != -1) {
                lock.lock();
                this->prev_impedance_cv_.wait(lock, [&]{return this->state_ == State::IDLE;});
            }
        }
    }
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    return this->prev_impedance_;
}

std::pair<float*, int> ABMHeadset::get_raw_data(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::ACQUISITION) {
        if (this->state_ != State::IDLE) this->force_idle_();
        if (this->start_acquisition_() == -1) {
            *this << "Failed to start acquisition" << std::endl;
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
    // Log Path
    if (!log_path.empty()) {
        set_log_path(log_path.wstring());
        ensure_logging();
    }
    
    // Device Connection Callback
    devcb = std::bind(&ABMHeadset::callback_device_info_, this, std::placeholders::_1);
    if (!RegisterCallbackDeviceDetectionInfo(&devcb_trampoline)) {
        *this << "Failed to register device detection callback" << std::endl;
        return 1;
    }

    // Device Status Callback
    statcb = std::bind(&ABMHeadset::callback_status_info_, this, std::placeholders::_1);
    if (!RegisterCallbackOnStatusInfo(&statcb_trampoline)) {
        *this << "Failed to register the status callback" << std::endl;
        return 2;
    }

    impcb = std::bind(&ABMHeadset::callback_impedance_finished_, this, std::placeholders::_1, std::placeholders::_2);
    impelcb = std::bind(&ABMHeadset::callback_electrode_impedance_, this, std::placeholders::_1, std::placeholders::_2);
    if (!RegisterCallbackImpedanceElectrodeFinishedA(impelcb_trampoline)) {
        *this << "Failed to register impedance elctrode callback" << std::endl;
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
    
    *this << "Config Path set to: " << config_path << std::endl;
    int result = SetConfigPath(config_path.wstring().data());
    if (result != 1) {
        *this << "Failed to set config path" << std::endl;
        this->disconnect_();
        return 5;
    }

    // Connect to the device
    if (!this->connect_()) {
        *this << "Failed to connect to the device" << std::endl;
        return 6;
    }

    this->initialized_ = true;
    return 0;
}

void ABMHeadset::callback_device_info_(std::wstring const& message) {
    *this << L"Device Message: " << message << std::endl;
}

void ABMHeadset::callback_electrode_impedance_(std::string const& channel, float const& impedance) {
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    this->prev_impedance_[channel] = impedance;
}

void ABMHeadset::callback_impedance_finished_(ELECTRODE* pEl, int& i) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ == State::IMPEDANCE) this->state_ = State::IDLE;
    {
        std::lock_guard<std::mutex> lock(this->connected_mutex_);

    }
    this->disconnect_();
    this->prev_impedance_cv_.notify_all();
}

void ABMHeadset::callback_monitoring_(CHANNEL_INFO* ch, int& n) {
    std::lock_guard<std::mutex> lock(this->prev_monitoring_mutex_);
    for (int i = 0; i < n; ch++) {
        std::wstring wchname = ch->chName;
        std::string chname(wchname.begin(), wchname.end());
        this->prev_monitoring_[chname] = ch->bTechnicalInfo;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
        this->state_ = State::IDLE;
    }
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
    return ret;
}

void ABMHeadset::force_idle_(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ == State::IDLE) return;
    switch (this->state_) {
        case State::ACQUISITION:
            lock.unlock();
            *this << "Stopping Acquisition" << std::endl;
            this->stop_acquisition_();
            break;
        case State::TECHNICAL:
            lock.unlock();
            *this << "Stopping Technical monitoring" << std::endl;
            this->stop_technical_();
            break;
        case State::IMPEDANCE:
            lock.unlock();
            *this << "Stopping Impedance" << std::endl;
            this->stop_impedance_();
            break;
    }
    return;
}

std::wostream& ABMHeadset::operator<<(std::wstring const& in) {
    std::lock_guard<std::mutex> lock(this->cout_mutex_);
    return std::wcout << in;
}
std::ostream& ABMHeadset::operator<<(std::string const& in) {
    std::lock_guard<std::mutex> lock(this->cout_mutex_);
    return std::cout << in;
}

int ABMHeadset::start_acquisition_(void) {
    *this << "Acquisition func" << std::endl;
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    *this << "Checking Idle" << std::endl;
    if (this->state_ != State::IDLE) return -1;
    *this << "Checking start session" << std::endl;
    if (this->start_session_() != INIT_SESSION_OK) return -1;
    // Electrode Names
    if (this->electrode_names_.size() == 0) {
        _CHANNELMAP_INFO channel_map;
        if (!GetChannelMapInfo(channel_map)) {
            *this << "Failed to get channel data" << std::endl;
            return 7;
        }
        for (int i = 0; i < 24; i++) {
            if (!channel_map.stEEGChannels.bChUsed[i]) continue;
            this->electrode_names_.emplace_back(channel_map.stEEGChannels.cChName[i]);
        }
    }
    *this << "Checking start acquisition" << std::endl;
    if (StartAcquisitionForCurrentConnection() != ACQ_STARTED_OK) return -1;
    *this << "Started Acquisition!" << std::endl;
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
        *this << "Failed to connect" << std::endl;
        return -1;
    }
    *this << "start_session_: ";
    int retval = InitSessionForCurrentConnection(device, session_type, -1, false);
    *this << std::to_string(retval) << std::endl;
    return retval;
}

int ABMHeadset::start_technical_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::IDLE) return -1;
    if (this->start_session_() != 1) return -1;
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