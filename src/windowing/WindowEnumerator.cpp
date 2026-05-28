#include "WindowEnumerator.h"

std::vector<WindowInfo> WindowEnumerator::enumerate() const {
    std::vector<WindowInfo> windows;

    EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(&windows));

    return windows;
}

BOOL CALLBACK WindowEnumerator::enumWindowsCallback(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);

    if (!isManageableWindow(hwnd)) return TRUE;

    WindowInfo info;
    info.hwnd = hwnd;
    info.rect = {};
    info.title = getWindowTitle(hwnd);

    GetWindowRect(hwnd, &info.rect);
    windows->push_back(info);

    return TRUE;
}

bool WindowEnumerator::isManageableWindow(HWND hwnd)
{
    if (!IsWindowVisible(hwnd))
        return false;

    if (GetWindow(hwnd, GW_OWNER) != nullptr)
        return false;

    if (GetWindowTextLengthW(hwnd) <= 0)
        return false;

    RECT rect{};
    if (!GetWindowRect(hwnd, &rect))
        return false;

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    return width > 0 && height > 0;
}

std::wstring WindowEnumerator::getWindowTitle(HWND hwnd)
{
    const int length = GetWindowTextLengthW(hwnd);
    if (length <= 0)
        return L"";

    std::wstring title(length, L'\0');
    GetWindowTextW(hwnd, title.data(), length + 1);

    return title;
}