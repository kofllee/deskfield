#pragma once

#include "WindowInfo.h"

#include <vector>

class WindowEnumerator {
public:
    std::vector<WindowInfo> enumerate() const;

private:
    static BOOL CALLBACK enumWindowsCallback(HWND hwnd, LPARAM lParam);

    static bool isManageableWindow(HWND hwnd);
    static std::wstring getWindowTitle(HWND hwnd);
};
