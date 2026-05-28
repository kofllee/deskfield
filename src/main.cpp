#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"

#include <windows.h>

#include <iostream>

int main() {
    SetProcessDPIAware();

    WindowEnumerator enumerator;
    WindowController controller;

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
            << L" | pid=" << window.processId
            << L" | class=" << window.className
            << L" | title=" << window.title
            << L" | x=" << x
            << L" y=" << y
            << L" w=" << width
            << L" h=" << height
            << L"\n";

        if (window.className == L"Notepad") {
            RECT baseRect = controller.getBestWindowRect(window.hwnd);

            const int width = baseRect.right - baseRect.left;
            const int height = baseRect.bottom - baseRect.top;

            RECT moved{};
            moved.left = baseRect.left + 100;
            moved.top = baseRect.top + 200;
            moved.right = moved.left + width;
            moved.bottom = moved.top + height;

            std::wcout << L"\nMoving Notepad...\n";

            controller.moveWindow(window.hwnd, moved);
            controller.bringToForeground(window.hwnd);

            return 0;
        }
    }

    std::wcout << L"\nNotepad was not found.\n";
    return 0;
}