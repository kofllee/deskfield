#pragma once

#include "WindowSnapshot.h"

#include <vector>

class WindowEnumerator {
public:
    std::vector<WindowSnapshot> enumerate() const;

private:
    static BOOL CALLBACK enumWindowsCallback(HWND hwnd, LPARAM lParam);

    static bool isManageableWindow(HWND hwnd);

    static std::wstring getWindowTitle(HWND hwnd);
    static std::wstring getWindowClassName(HWND hwnd);

    static bool isCloaked(HWND hwnd);
    static bool hasWindowStyle(HWND hwnd, LONG_PTR style);
    static bool hasWindowExStyle(HWND hwnd, LONG_PTR style);
};
