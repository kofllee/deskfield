#pragma once

#include <windows.h>
#include <string>

struct WindowInfo {
    HWND hwnd{};
    RECT rect{};
    std::wstring title;
};
