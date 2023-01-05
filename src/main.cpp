#define UNICODE
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>

#undef UNICODE
#include "AbmSdkInclude.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  _DEVICE_INFO* device_info = GetDeviceInfoKeepConnection(NULL);
  if (device_info == NULL) {
    MessageBox(NULL, L"Failed to find device", L"ERROR", MB_ICONERROR | MB_OK);
    CloseCurrentConnection();
    return 0;
  }
  
  // Print the device name
  std::basic_string<TCHAR> alert = std::basic_string<TCHAR>(_T(L"Device: ")) + device_info->chDeviceName;
  MessageBox(NULL, alert.c_str(), L"INFO", MB_OK);

  // Point to configuration files
  std::wstring config_path(L"C:/ABM/B-Alert/Config/");
  int result = SetConfigPath(config_path.data());
  if (result != 1) {
      MessageBox(NULL, L"Failed to set config path", L"ERROR", MB_ICONERROR | MB_OK);
      CloseCurrentConnection();
      return 0;
  }
  MessageBox(NULL, L"Set Config Path", L"INFO", MB_OK);
  
  // Initialize the session
  if(InitSessionForCurrentConnection(ABM_DEVICE_X24Standard, ABM_SESSION_RAW, -1, false) != INIT_SESSION_OK) {
    MessageBox(NULL, L"Failed to initialize EEG session", L"ERROR", MB_ICONERROR | MB_OK);
    CloseCurrentConnection();
    return 0;
  }
  MessageBox(NULL, L"Session initialized", L"INFO", MB_OK);
  
  // Start Data Acquisition
  if(StartAcquisitionForCurrentConnection() != ACQ_STARTED_OK) {
    MessageBox(NULL, L"Failed to begin data acquisition", L"ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  MessageBox(NULL, L"Session started", L"INFO", MB_OK);
 
  int n_samples = 5;
  float* data = GetRawData(n_samples);
  if (data == NULL) {
    MessageBox(NULL, L"Failed to collect data", L"ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  MessageBox(NULL, L"Data Collected!", L"INFO", MB_OK);
  std::wstring data_message = L"";
  for (int i = 0; i < 5; i++) {
    data_message += std::to_wstring(data[i]) + L", ";
  }
  MessageBox(NULL, data_message.c_str(), L"Data", MB_OK); 

  StopAcquisitionKeepConnection();
  MessageBox(NULL, L"Hello, Message Box!", L"Message", MB_OK);
  
  CloseCurrentConnection();
  return 0;
}
