#define UNICODE
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>

#undef UNICODE
#include "AbmSdkInclude.h"

void print_sample(float* sample, int n_channels) {
    char time[256] = { 0 };
    sprintf(time, "%d:%d:%d:%f", sample[2], sample[3], sample[4], sample[5]);
    std::cout << "Epoch: " << sample[0] << "\t"
        << "Offset: " << sample[1] << "\t"
        << "Time: " << time << "\t";

    /*for (int ch = 0; ch < n_channels; ch++) {
        float value = sample[ch + 6];
        std::cout << "ch" << ch << ": " << value << "\t";
    }*/
    std::cout << "POz: " << sample[16 + 6];
    std::cout << std::endl;
    return;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  if (!AttachConsole(-1)) {
    MessageBox(NULL, L"Failed to attach console", L"ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  FILE* p;
  freopen_s(&p, "CONOUT$", "w", stdout);
  std::cout << "Attached!" << std::endl;
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
 
  while (true) {
      int n_samples = 1;
      float* data = GetRawData(n_samples);
      if (data == NULL) {
          continue;
      }

      print_sample(data, device_info->nNumberOfChannel);
  }

  StopAcquisitionKeepConnection();
  MessageBox(NULL, L"Hello, Message Box!", L"Message", MB_OK);
  
  CloseCurrentConnection();
  std::cout << "Hello World" << std::endl;
  return 0;
}
