#ifndef INCLUDE_HEADSET_ABMHEADSET_HPP_
#define INCLUDE_HEADSET_ABMHEADSET_HPP_
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "sdk/sdk.hpp"
#include "headset/state.hpp"
#ifdef __PYBIND11__
#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
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
    ~ABMHeadset(void);
    float get_battery_percentage(void);
    std::vector<std::string> get_data_keys(void);
    std::map<std::string, float> const& get_impedance_values(std::vector<std::string> electrodes = {});
    std::pair<float*, int> get_raw_data(void);
    std::map<std::string, std::vector<float>> get_raw_data_vector(void);
    int init(std::filesystem::path log_path = "");
  private:
    void callback_device_info_(std::wstring const&);
    void callback_electrode_impedance_(std::string const& channel, float const& impedance);
    void callback_impedance_finished_(ELECTRODE* pEl, int& i);
    void callback_monitoring_(CHANNEL_INFO* ch, int& n);
    void callback_status_info_(_STATUS_INFO* status_info);
    int connect_(void);
    int disconnect_(void);
    void force_idle_(void);
	ABMHeadset& operator<<(std::string const& in);
    int start_acquisition_(void);
    int start_impedance_(std::vector<std::string> const& electrodes);
    int start_session_(int device = ABM_DEVICE_X24Standard, int session_type = ABM_SESSION_RAW);
    int start_technical_(void);
    void stop_acquisition_(void);
    void stop_impedance_(void);
    void stop_technical_(void);
    std::mutex battery_mutex_;
    float battery_percentage_;
    std::mutex connected_mutex_;
    bool connected_ = false;
    std::mutex cout_mutex_;
    std::string device_name_;
    std::vector<std::string> electrode_names_;
    bool initialized_ = false;
    int num_channels_ = 0;
    std::map<std::string, float> prev_impedance_;
    std::condition_variable_any prev_impedance_cv_;
    std::mutex prev_impedance_mutex_;
    std::map<std::string, bool> prev_monitoring_;
    std::mutex prev_monitoring_mutex_;
    State state_ = State::IDLE;
    std::recursive_mutex state_mutex_;
};

#endif  /* INCLUDE_HEADSET_ABMHEADSET_HPP_ */