#define UNICODE
#include <windows.h>
#include <tchar.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

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

int setup_console(void) {
  if (!AttachConsole(-1)) {
    MessageBox(NULL, L"Failed to attach console", L"ERROR", MB_ICONERROR | MB_OK);
    return 0;
  }

  FILE* p;
  freopen_s(&p, "CONOUT$", "w", stdout);
  return 1;
}

std::function<void(std::wstring, float)> g_cb;

void __stdcall impcallback(TCHAR* chname, float imp) {
  g_cb(chname, imp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  if (!setup_console()) {
    return 1;
  }

  std::cout << "Attached!" << std::endl;
  _DEVICE_INFO* device_info = GetDeviceInfoKeepConnection(NULL);
  if (device_info == NULL) {
    std::cout << "Failed to find device" << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  
  // Print the device name
  std::basic_string<TCHAR> alert = std::basic_string<TCHAR>(_T(L"Device: ")) + device_info->chDeviceName;
  std::wcout << "Found Device: " << alert << std::endl;

  // Point to configuration files
  std::wstring config_path(L"C:/ABM/B-Alert/Config/");
  int result = SetConfigPath(config_path.data());
  if (result != 1) {
    std::cout << "Failed to set config path" << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  std::cout << "Set Config Path" << std::endl;
  
  // Initialize the session
  int init_ret = InitSessionForCurrentConnection(ABM_DEVICE_X24Standard, ABM_SESSION_RAW, -1, false);
  if(init_ret != INIT_SESSION_OK) {
    std::cout << "Failed to initialize EEG session. Error Code: " << init_ret << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  std::cout << "Session initializd" << std::endl;

  // Register Callback
  std::vector<std::pair<std::wstring, float>> imp_data;
  g_cb = [&](std::wstring n, float i) {
    std::wcout << L"Impdance for channel " << n << ": " << i << std::endl;
    imp_data.push_back(
      std::pair<std::wstring, float>(n, i)
    );
  };
  if (!RegisterCallbackImpedanceElectrodeFinishedA(impcallback)) {
    std::cout << "Failed to register electrode impedance callback" << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  std::cout << "Callback regestered" << std::endl;

  TCHAR* electrode_list[3] = {L"C3", L"Cz", L"C4"};
  int retval = CheckSelectedImpedancesForCurrentConnection(NULL, electrode_list, 3, 0);
  switch (retval) {
    case 0:
      std::cout << "Impedance checking request was ignored. Session needs to be initialized first" << std::endl;
      break;
    case -1:
      std::cout << "Failed to start checking impedance" << std::endl;
      break;
    case 1:
      std::cout << "Impedance checking started" << std::endl;
  }

  std::cout << "Waiting for callbacks" << std::endl;
  while (imp_data.size() < 4) { Sleep(10); }
  Sleep(1000);

  std::cout << "Trying impedance check again..." << std::endl;

  // ReInitialize the session
  GetDeviceInfoKeepConnection(NULL);
  init_ret = InitSessionForCurrentConnection(ABM_DEVICE_X24Standard, ABM_SESSION_RAW, -1, false);
  if(init_ret != INIT_SESSION_OK) {
    std::cout << "Failed to initialize EEG session. Error Code: " << init_ret << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  std::cout << "Session initializd" << std::endl;

  // Register Callback
  imp_data.clear();
  if (!RegisterCallbackImpedanceElectrodeFinishedA(impcallback)) {
    std::cout << "Failed to register electrode impedance callback" << std::endl;
    CloseCurrentConnection();
    return 1;
  }
  std::cout << "Callback regestered" << std::endl;

  retval = CheckSelectedImpedancesForCurrentConnection(NULL, electrode_list, 3, 0);
  switch (retval) {
    case 0:
      std::cout << "Impedance checking request was ignored. Session needs to be initialized first" << std::endl;
      break;
    case -1:
      std::cout << "Failed to start checking impedance" << std::endl;
      break;
    case 1:
      std::cout << "Impedance checking started" << std::endl;
  }

  std::cout << "Waiting for callbacks" << std::endl;
  while (imp_data.size() < 4) { Sleep(10); }

  /*
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

  StopAcquisitionKeepConnection();*/
  
  CloseCurrentConnection();
  std::cout << "Done" << std::endl;
  return 0;
}
