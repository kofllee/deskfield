#include "WindowEnumerator.h"

#include <dwmapi.h>

#include <array>

std::vector<WindowSnapshot> WindowEnumerator::enumerate() const {
    std::vector<WindowSnapshot> windows;

    EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(&windows));

    return windows;
}

BOOL CALLBACK WindowEnumerator::enumWindowsCallback(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<WindowSnapshot>*>(lParam);

    if (!isManageableWindow(hwnd)) return TRUE;

    WindowSnapshot info;
    info.hwnd = hwnd;
    info.title = getWindowTitle(hwnd);
    info.className = getWindowClassName(hwnd);

    GetWindowRect(hwnd, &info.rect);
    
    info.normalRect = info.rect;

    WINDOWPLACEMENT placement{};
    placement.length = sizeof(WINDOWPLACEMENT);

    if (GetWindowPlacement(hwnd, &placement)) {
        info.normalRect = placement.rcNormalPosition;
    }

    GetWindowThreadProcessId(hwnd, &info.processId);

    info.visible = IsWindowVisible(hwnd) != FALSE;
    info.minimized = IsIconic(hwnd) != FALSE;
    info.maximized = IsZoomed(hwnd) != FALSE;
    info.cloaked = isCloaked(hwnd);
    info.toolWindow = hasWindowExStyle(hwnd, WS_EX_TOOLWINDOW);
    info.appWindow = hasWindowExStyle(hwnd, WS_EX_APPWINDOW);

    windows->push_back(info);

    return TRUE;
}

bool WindowEnumerator::isManageableWindow(HWND hwnd){
    if (hwnd == nullptr)
        return false;

    if (!IsWindow(hwnd))
        return false;

    if (!IsWindowVisible(hwnd))
        return false;

    if (GetWindow(hwnd, GW_OWNER) != nullptr)
        return false;

    if (hasWindowExStyle(hwnd, WS_EX_TOOLWINDOW))
        return false;

    if (isCloaked(hwnd))
        return false;

    const std::wstring title = getWindowTitle(hwnd);
    if (title.empty())
        return false;

    RECT rect{};
    if (!GetWindowRect(hwnd, &rect))
        return false;

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0)
        return false;

    const std::wstring className = getWindowClassName(hwnd);

    if (className == L"Progman")
        return false;
    if (className == L"WorkerW")
        return false;
    if (className == L"Shell_TrayWnd")
        return false;
    if (className == L"DV2ControlHost")
        return false;
    if (className == L"SunAwtFrame")
        return false;

    return true;
}

std::wstring WindowEnumerator::getWindowTitle(HWND hwnd) {
    const int length = GetWindowTextLengthW(hwnd);
    if (length <= 0)
        return L"";

    std::wstring title(static_cast<size_t>(length) + 1, L'\0');
    const int copied = GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));

    if (copied <= 0) {
        return L"";
    }

    title.resize(static_cast<size_t>(copied));
    return title;
}

std::wstring WindowEnumerator::getWindowClassName(HWND hwnd) {
    std::array<wchar_t, 256> buffer{};
    const int copied = GetClassNameW(hwnd, buffer.data(), static_cast<int>(buffer.size()));

    if (copied <= 0) {
        return L"";
    }

    return std::wstring(buffer.data(), static_cast<size_t>(copied));
}

bool WindowEnumerator::isCloaked(HWND hwnd) {
    BOOL cloaked = FALSE;
    const HRESULT result = DwmGetWindowAttribute(
        hwnd,
        DWMWA_CLOAKED,
        &cloaked,
        sizeof(cloaked)
    );

    return SUCCEEDED(result) && cloaked != FALSE;
}

bool WindowEnumerator::hasWindowStyle(HWND hwnd, LONG_PTR style) {
    const LONG_PTR value = GetWindowLongPtrW(hwnd, GWL_STYLE);
    return (value & style) != 0;
}

bool WindowEnumerator::hasWindowExStyle(HWND hwnd, LONG_PTR style) {
    const LONG_PTR value = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    return (value & style) != 0;
}