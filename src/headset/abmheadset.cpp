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
#include "headset/hwresult.hpp"
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
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::DISCONNECTED) {
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
    if (this->electrode_names_.size() != this->num_channels_) {
        this->start_session_();
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
    // Check electrodes requested
    if (electrodes.size() == 0) {
        std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
        return this->prev_impedance_;
    }
    // Check state
    {
#ifdef __PYBIND11__
        py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
        std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
        if (this->state_ != State::IMPEDANCE) {
            HWResult ret = this->start_impedance_(electrodes);
            if (ret != HWResult::Success) {
                this->print(std::string("Failed to start impedance check: ") + std::to_string(ret));
                return std::map<std::string, float>();
            }
            this->prev_impedance_cv_.wait(lock, [&]{return this->state_ <= State::IDLE;});
        }
    }
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    return this->prev_impedance_;
}

int const ABMHeadset::get_state(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    return this->state_;
}

std::map<std::string, bool>const& ABMHeadset::get_technical_data(void) {
    {
#ifdef __PYBIND11__
        py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
        std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
        if (this->state_ != State::TECHNICAL) {
            this->print("Starting technical check");
            HWResult ret = this->start_technical_();
            if (ret != HWResult::Success) {
                this->print(std::string("Failed to start technical check: ") + std::to_string(ret));
                return std::map<std::string, bool>();
            }
            this->prev_monitoring_cv_.wait(lock, [&]{return this->state_ <= State::IDLE;});
        }
    }
    std::lock_guard<std::mutex> lock(this->prev_monitoring_mutex_);
    return this->prev_monitoring_;
}

std::pair<float*, int> ABMHeadset::get_raw_data(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ != State::ACQUISITION) {
#ifdef __PYBIND11__
        py::gil_scoped_release release;
#endif  /* __PYBIND11__ */
        HWResult ret = this->start_acquisition_();
        if (ret != HWResult::Success) {
            this->print(std::string("Failed to start acquisition: ") + std::to_string(ret));
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

#ifdef __PYBIND11__
py::array_t<float> ABMHeadset::get_raw_npdata(bool block) {
  using namespace std::chrono_literals;
    std::pair<float*, int> data = this->get_raw_data();
    while (block && data.second < 1) {
      std::this_thread::sleep_for(40ms);
      data = this->get_raw_data();
    }
    int n_cols = this->num_channels_ + 6;
    return py::array_t<float>(
        { data.second, n_cols },
        { sizeof(float) * n_cols, sizeof(float) },
        data.first
    );
}
#endif /* __PYBIND11__ */

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
        return 5;
    }

    // Connect to the device
    HWResult ret = this->connect_();
    if (ret != HWResult::Success) {
        this->print(
            std::string("Failed to connect to the device")
            + std::to_string(ret)
        );
        return 6;
    }

    return 0;
}

bool ABMHeadset::set_destination_file(std::filesystem::path const& destination_file) {
    this->destination_file_ = destination_file;
	this->destination_file_.make_preferred();
	bool is_directory = this->destination_file_.extension().empty();
	if (is_directory) return false;
	std::filesystem::create_directories(this->destination_file_.parent_path());
    return SetDestinationFile(this->destination_file_.wstring().data());
}

void ABMHeadset::stop_acquisition(void) {
  this->stop_acquisition_();
}

void ABMHeadset::callback_device_info_(std::wstring const& message) {
    this->print(std::wstring(L"Device Message: ") + message);
}

void ABMHeadset::callback_electrode_impedance_(std::string const& channel, float const& impedance) {
    std::lock_guard<std::mutex> lock(this->prev_impedance_mutex_);
    this->prev_impedance_[channel] = impedance;
}

void ABMHeadset::callback_impedance_finished_(ELECTRODE* pEl, int& i) {
#ifdef __PYBIND11__
    py::gil_scoped_acquire acquire;
#endif  /* __PYBIND11__ */
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    this->print("Impedance is finished");
    if (this->state_ == State::IMPEDANCE) this->state_ = State::IDLE;
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

HWResult ABMHeadset::connect_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ > State::DISCONNECTED) return HWResult::Success;
    _DEVICE_INFO* device_info = GetDeviceInfoKeepConnection(NULL);
    if (device_info == nullptr) return HWResult::EConnection;
    if (this->device_name_.empty()) {
        std::wstring wdevicename = device_info->chDeviceName;
        this->device_name_.assign(wdevicename.begin(), wdevicename.end());
        this->num_channels_ = device_info->nNumberOfChannel;
    }
    this->state_ = State::IDLE;
    return HWResult::Success;
}

int ABMHeadset::disconnect_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ == State::DISCONNECTED) return 0;
    if (this->state_ > State::IDLE) this->force_idle_();
    CloseCurrentConnection();
    this->state_ = State::DISCONNECTED;
    this->print("Disconnected");
    return 0;
}

void ABMHeadset::force_idle_(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ <= State::IDLE) return;
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
    this->state_ = State::IDLE;
    return;
}

HWResult ABMHeadset::start_acquisition_(void) {
    std::unique_lock<std::recursive_mutex> lock(this->state_mutex_);
    // Get electrode names
    if (this->electrode_names_.size() == 0) this->get_electrode_names();

    HWResult ret = this->start_session_();
    if (ret != HWResult::Success) return ret;
    
    int status = StartAcquisitionForCurrentConnection();
    if (status != ACQ_STARTED_OK) {
        if (status == ACQ_STARTED_NO) return HWResult::EFailed;
        else HWResult::EBadSeq;
    }
    this->state_ = State::ACQUISITION;
    return HWResult::Success;
}

HWResult ABMHeadset::start_impedance_(std::vector<std::string> const& electrodes) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    HWResult ret = this->start_session_();
    if (ret != HWResult::Success) return ret;
    
    std::vector<std::wstring> welectrodes;
    for (std::string const& s : electrodes) { welectrodes.emplace_back(s.begin(), s.end()); }
    std::vector<TCHAR*> electrode_names;
    for (std::wstring& w : welectrodes) { electrode_names.push_back(w.data()); }
    int status = CheckSelectedImpedancesForCurrentConnection(impcb_trampoline, electrode_names.data(), electrode_names.size(), 0);
    if (status != IMP_STARTED_OK) {
        if (status == IMP_STARTED_NO) return HWResult::EFailed;
        else HWResult::EBadSeq;
    }
    this->state_ = State::IMPEDANCE;
    return HWResult::Success;
}

HWResult ABMHeadset::start_session_(int device, int session_type) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    if (this->state_ == State::INITIALIZED) return HWResult::Success;
    if (this->state_ != State::IDLE) {
        if (this->connect_() != HWResult::Success) return HWResult::EConnection;
        this->force_idle_();
    }
    int ret = InitSessionForCurrentConnection(device, session_type, -1, false);
    if (ret != INIT_SESSION_OK) {
        if (ret == INIT_SESSION_NO) return HWResult::EFailed;
        else return HWResult::EBadSeq;
    }
    this->state_ = State::INITIALIZED;
    this->print("Session started");
    return HWResult::Success;
}

HWResult ABMHeadset::start_technical_(void) {
    std::lock_guard<std::recursive_mutex> lock(this->state_mutex_);
    HWResult ret = this->start_session_(1, 1);
    if (ret != HWResult::Success) return ret;
    int status = TechnicalMonitoring(&techcb_trampoline, 15, NULL);
    if (status != TM_STARTED_OK) {
        if (status == TM_STARTED_NO) return HWResult::EFailed;
        else return HWResult::EBadSeq;
    }
    this->state_ = State::TECHNICAL;
    return HWResult::Success;
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