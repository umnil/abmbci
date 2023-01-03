#include <windows.h>
#include <iostream>
#include <string>

extern "C" {
#include "Athena.h"
}

typedef _DEVICE_INFO* (*PGetDeviceInfo)(_DEVICE_INFO*);
typedef float* (*PGetRawData)(int&);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // Load Module Procedures
  HMODULE module = LoadLibrary("ABM_Athena.dll");
  if (module == NULL) {
    MessageBox(NULL, "Failed to load the Athena Library", "ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  PGetDeviceInfo pGetDeviceInfo = (PGetDeviceInfo)GetProcAddress(module, "GetDeviceInfo");
  PGetRawData pGetRawData = (PGetRawData)GetProcAddress(module, "GetRawData");

  _DEVICE_INFO* device_info = pGetDeviceInfo(NULL);
  if (device_info != NULL) {
    char const* device_name = device_info->chDeviceName;
    std::string alert = std::string("Device: ") + device_name;
    MessageBox(NULL, alert.c_str(), "INFO", MB_OK);
  }

  int n_samples = 5;
  float* data = pGetRawData(n_samples);
  if (data == NULL) {
    MessageBox(NULL, "Failed to collect data", "ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  std::string data_message = "";
  for (int i = 0; i < 5; i++) {
    data_message += std::to_string(data[i]) + ", ";
  }
  MessageBox(NULL, data_message.c_str(), "Data", MB_OK); 
  MessageBox(NULL, "Hello, Message Box!", "Message", MB_OK);
  std::cout << "Hello, World!" << std::endl;
  return 0;
}
