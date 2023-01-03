#include <windows.h>
#include <iostream>
#include <string>

extern "C" {
#include "Athena.h"
}

typedef _DEVICE_INFO* (*PGetDeviceInfo)(_DEVICE_INFO*);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  HMODULE module = LoadLibrary("ABM_Athena.dll");
  if (module == NULL) {
    MessageBox(NULL, "Failed to load the Athena Library", "ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }
  PGetDeviceInfo pGetDeviceInfo = (PGetDeviceInfo)GetProcAddress(module, "GetDeviceInfo");
  _DEVICE_INFO* device_info = pGetDeviceInfo(NULL);
  if (device_info != NULL) {
    char const* device_name = device_info->chDeviceName;
    std::string alert = std::string("Device: ") + device_name;
    MessageBox(NULL, alert.c_str(), "INFO", MB_OK);
  }
  MessageBox(NULL, "Hello, Message Box!", "Message", MB_OK);
  std::cout << "Hello, World!" << std::endl;
  return 0;
}
