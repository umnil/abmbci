#ifndef INCLUDE_HEADSET_ABMHEADSET_HPP_
#define INCLUDE_HEADSET_ABMHEADSET_HPP_
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "sdk/sdk.hpp"
#include "headset/hwresult.hpp"
#include "headset/state.hpp"
#ifdef __PYBIND11__
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;
#endif  /* __PYBIND11__ */
extern std::function<void(std::wstring const&)> devcb;
extern std::function<void(_STATUS_INFO*)> statcb;
extern std::function<void(ELECTRODE*, int&)> impcb;
extern std::function<void(std::string const&, float const&)> impelcb;
extern std::function<void(CHANNEL_INFO* chInfo, int& i)> techcb;
void __stdcall devcb_trampline(wchar_t* message, int i);
void __stdcall statcb_trampoline(_STATUS_INFO* status_info);
void __stdcall impcb_trampoline(ELECTRODE* pEl, int& i);
void __stdcall impelcb_trampoline(char* ch, float i);
void __stdcall techcb_trampoline(CHANNEL_INFO* ch, int& i);
class ABMHeadset {
  public:
    ABMHeadset(HeadsetType headset_type = HeadsetType::X24_QEEG);
    ~ABMHeadset(void);
    float get_battery_percentage(void);
    std::vector<std::string> get_data_keys(void);
    std::vector<std::string> get_electrode_names(void);
    std::map<std::string, float> const& get_impedance_values(std::vector<std::string> electrodes = {});
    int const get_state(void);
    std::map<std::string, bool> const& get_technical_data(void);
    std::pair<float*, int> get_raw_data(void);
    std::map<std::string, std::vector<float>> get_raw_data_vector(void);
#ifdef __PYBIND11__
    py::array_t<float> get_raw_npdata(bool block = false);
#endif /* __PYBIND11__ */
    int init(std::filesystem::path log_path = "");
    bool set_destination_file(std::filesystem::path const& destination_pth = "");
    void stop_acquisition(void);
  private:
    void callback_device_info_(std::wstring const&);
    void callback_electrode_impedance_(std::string const& channel, float const& impedance);
    void callback_impedance_finished_(ELECTRODE* pEl, int& i);
    void callback_monitoring_(CHANNEL_INFO* ch, int& n);
    void callback_status_info_(_STATUS_INFO* status_info);
    HWResult connect_(void);
    int disconnect_(void);
    void force_idle_(void);
    template<class T>
    void print(std::basic_string<T> const& in);
    template<class T> requires(std::is_integral<T>::value)
    void print(T const* in);
    HWResult start_acquisition_(void);
    HWResult start_impedance_(std::vector<std::string> const& electrodes);
    HWResult start_session_(int device = -1, int session_type = ABM_SESSION_RAW);
    HWResult start_technical_(void);
    void stop_acquisition_(void);
    void stop_impedance_(void);
    void stop_technical_(void);
    std::mutex battery_mutex_;
    float battery_percentage_;
    std::mutex cout_mutex_;
    std::filesystem::path destination_file_;
    HeadsetType headset_type_;
    std::string device_name_;
    std::vector<std::string> electrode_names_;
    int num_channels_ = 0;
    std::map<std::string, float> prev_impedance_;
    std::condition_variable_any prev_impedance_cv_;
    std::mutex prev_impedance_mutex_;
    std::map<std::string, bool> prev_monitoring_;
    std::condition_variable_any prev_monitoring_cv_;
    std::mutex prev_monitoring_mutex_;
    State state_ = State::DISCONNECTED;
    std::recursive_mutex state_mutex_;
};


template<class T>
void ABMHeadset::print(std::basic_string<T> const& in) {
  std::string out(in.begin(), in.end());
  std::lock_guard<std::mutex> lock(this->cout_mutex_);
#ifdef __PYBIND11__
  py::gil_scoped_acquire acquire;
  py::print(out);
#else  /* __PYBIND11__ */
  std::cout << out << std::endl;;
#endif  /* __PYBIND11__ */
}

template<class T> requires(std::is_integral<T>::value)
void ABMHeadset::print(T const* in) {
  this->print(std::string(in));
}
#endif  /* INCLUDE_HEADSET_ABMHEADSET_HPP_ */