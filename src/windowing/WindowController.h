#pragma once

#include <windows.h>

class WindowController {
public:
    bool moveWindow(HWND hwnd, const RECT& rect) const;
    bool bringToForeground(HWND hwnd) const;
    bool restoreWindow(HWND hwnd) const;

    RECT getBestWindowRect(HWND hwnd) const;

};
