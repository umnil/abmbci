#include <filesystem>
#include <iostream>
#include <string>

#define LOGPATH_ADDR 0x100a4208

std::wstring const get_log_path(void) {
    return std::wstring(
        reinterpret_cast<wchar_t*>(LOGPATH_ADDR)
    );
}

void set_log_path(std::wstring path) {
    auto end = path.end();
    if (path.size() > 0x400) {
        end = path.begin() + (0x400 - 1);
    }
    std::copy(path.begin(), end, reinterpret_cast<wchar_t*>(LOGPATH_ADDR));
}

void ensure_logging(void) {
    std::wstring log_path = get_log_path();
    if (!std::filesystem::exists(log_path)) {
        std::filesystem::create_directories(log_path);
    }
    return;
}