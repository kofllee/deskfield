#include "WindowController.h"

RECT WindowController::getBestWindowRect(HWND hwnd) const {
    RECT rect = {};

    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return rect;
    }

    WINDOWPLACEMENT placement{};
    placement.length = sizeof(WINDOWPLACEMENT);

    if (GetWindowPlacement(hwnd, &placement)) {
        if (placement.showCmd == SW_SHOWMINIMIZED || placement.showCmd == SW_SHOWMAXIMIZED) {
            return placement.rcNormalPosition;
        }
    }

    GetWindowRect(hwnd, &rect);
    return rect;
}

bool WindowController::moveWindow(HWND hwnd, const RECT& rect) const {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return false;
    }

    const int x = rect.left;
    const int y = rect.top;
    const int w = rect.right - rect.left;
    const int h = rect.bottom - rect.top;

    if (w <= 0 || h <= 0) {
        return false;
    }

    restoreWindow(hwnd);

    const BOOL result = SetWindowPos(
        hwnd,
        HWND_TOP,
        x,
        y,
        w,
        h,
        SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS
    );


    return result;
}

bool WindowController::bringToForeground(HWND hwnd) const {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return false;
    }

    restoreWindow(hwnd);

    BringWindowToTop(hwnd);
    return SetForegroundWindow(hwnd) != FALSE;
}

bool WindowController::restoreWindow(HWND hwnd) const {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return false;
    }

    if (IsIconic(hwnd) || IsZoomed(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    return true;
}
