#include <windows.h>
#include <iostream>

extern "C" {
#include "Athena.h"
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  _DEVICE_INFO* device_info = GetDeviceInfo(NULL);
  MessageBox(NULL, "Hello, Message Box!", "Message", MB_OK);
  std::cout << "Hello, World!" << std::endl;
  return 0;
}
