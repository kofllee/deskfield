#pragma once

#include <windows.h>
#include <string>

struct WindowSnapshot {
    HWND hwnd{};
    DWORD processId{};

    RECT rect{};
    RECT normalRect{};

    std::wstring title;
    std::wstring className;

    bool visible{};
    bool minimized{};
    bool maximized{};
    bool cloaked{};
    bool toolWindow{};
    bool appWindow{};
};
