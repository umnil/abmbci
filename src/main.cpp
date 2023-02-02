#include <windows.h>
#include <iostream>
#include "headset/abmheadset.hpp"

int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    if (!AttachConsole(-1)) {
        MessageBox(NULL, "Failed to attach console", "ERROR", MB_ICONERROR | MB_OK);
        return 2;
    }
    FILE* p;
    freopen_s(&p, "CONIN$", "r", stdin);
    freopen_s(&p, "CONOUT$", "w", stdout);
    std::cout << "Attached" << std::endl;

    MessageBox(NULL, (std::string("PID: ") + std::to_string(getpid())).data(), "HOLD", MB_ICONINFORMATION | MB_OK);

    std::filesystem::path log_path = "..\\log";
    ABMHeadset headset;
    if (headset.init(log_path) != 0) {
        std::cout << "Failed to start" << std::endl;
        return 1;
    }

    std::filesystem::path dest_file = "..\\dest\\test.ebs";
    if (!headset.set_destination_file(dest_file)) {
        std::cout << "Failed to set destination file" << std::endl;
        return 1;
    }
    
    std::cout << "Checking impedance" << std::endl;
    std::map<std::string, float> imp = headset.get_impedance_values({
        "C4",
        "C3",
        "Cz"
    });
    if (imp.size() < 1) {
        std::cout << "Failed to get impedance values" << std::endl;
        return 1;
    }
    for (std::pair<std::string, float> p : imp) {
        std::cout << "Electrode " << p.first << ": " << p.second << "Ω" << std::endl;
    }

    std::chrono::time_point start = std::chrono::system_clock::now();
    std::chrono::seconds dt(5);
    std::chrono::time_point cur = std::chrono::system_clock::now();
    auto elapsed = cur - start;
    while (elapsed < dt) {
        std::pair<float*, int> packets = headset.get_raw_data();
        int n_packets = packets.second;
        float* data = packets.first;
        int i = 0;
        if (data == NULL) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        for (int j = 0; j < n_packets; j++) {
            for (int k = 0; k < 30; k++, i++) {
                std::cout << data[i] << ", ";
            }
            std::cout << std::endl;
        }
        cur = std::chrono::system_clock::now();
        elapsed = cur - start;
    }

    std::cout << "Done" << std::endl;
    return 0;
}