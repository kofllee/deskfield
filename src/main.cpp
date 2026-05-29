#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

#include <chrono>
#include <iostream>
#include <thread>

static RECT getPrimaryWorkArea() {
    RECT workArea{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    return workArea;
}

int main() {
    SetProcessDPIAware();

    WindowEnumerator enumerator;
    WindowController controller;
    WorkspaceModel workspace;
    ViewportMapper mapper;

    CanvasCamera camera{};
    camera.x = 0.0;
    camera.y = 0.0;
    camera.zoom = 1.0;

    const auto windows = enumerator.enumerate();
    workspace.rebuildFromWindows(windows);

    std::wcout << L"Deskfield canvas prototype\n";
    std::wcout << L"Managed windows: " << workspace.windows().size() << L"\n";
    std::wcout << L"Use arrow keys to pan. Press Esc to exit.\n";

    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        constexpr double panSpeed = 24.0;

        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            camera.x += panSpeed;
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            camera.x -= panSpeed;
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            camera.y += panSpeed;
        }

        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            camera.y -= panSpeed;
        }

        static int syncCounter = 0;

        if (++syncCounter >= 15) {
            syncCounter = 0;

            const auto currentWindows = enumerator.enumerate();
            workspace.syncFromWindows(currentWindows, camera);
            workspace.updateNativeState(currentWindows);
        }

        const RECT workArea = getPrimaryWorkArea();

        for (const ManagedWindow& window : workspace.windows()) {
            if (window.hwnd == nullptr || !IsWindow(window.hwnd)) {
                continue;
            }

            if (window.state != ManagedWindowState::Normal) {
                continue;
            }

            const RECT screenRect = mapper.mapCanvasToScreen(
                window.canvasRect,
                camera,
                workArea
            );

            controller.moveWindow(window.hwnd, screenRect);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}