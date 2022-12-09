#include <windows.h>

#include "Athena.h"

#include <iostream>

int main() {
  _DEVICE_INFO* device_info = GetDeviceInfo(NULL);
  MessageBox(NULL, "Hello, Message Box!", "Message", MB_OK);
  std::cout << "Hello, World!" << std::endl;
  return 0;
}
