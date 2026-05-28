#include "windowing/WindowEnumerator.h"

#include <windows.h>
#include <iostream>

int main() {
    SetProcessDPIAware();

    WindowEnumerator enumerator;
    const auto windows = enumerator.enumerate();

    std::wcout << L"Deskfield window scan\n";
    std::wcout << L"Found windows: " << windows.size() << L"\n\n";

    for (const auto& window : windows) {
        const int x = window.rect.left;
        const int y = window.rect.top;
        const int width = window.rect.right - window.rect.left;
        const int height = window.rect.bottom - window.rect.top;

        std::wcout
            << L"HWND=" << window.hwnd
            << L" | title=" << window.title
            << L" | x=" << x
            << L" y=" << y
            << L" w=" << width
            << L" h=" << height
            << L"\n";
    }

    return 0;
}
