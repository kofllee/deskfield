#pragma once

#include <windows.h>
#include <string>

struct WindowInfo {
    HWND hwnd{};
    DWORD processId{};

    RECT rect{};

    std::wstring title;
    std::wstring className;

    bool visible{};
    bool minimized{};
    bool maximized{};
    bool cloaked{};
    bool toolWindow{};
    bool appWindow{};
};
