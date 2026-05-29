#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"
#include "features/LayoutApplier.h"
#include "overlay/OverlayWindow.h"

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
    LayoutApplier layoutApplier;
    OverlayWindow overlay;

    if (!overlay.create(getPrimaryWorkArea())) {
        std::wcout << L"Failed to create overlay. Error: " << GetLastError() << L"\n";
        return 1;
    }

    overlay.show();

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
            camera.x -= panSpeed;
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            camera.x += panSpeed;
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            camera.y -= panSpeed;
        }

        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            camera.y += panSpeed;
        }

        static int syncCounter = 0;

        const auto currentWindows = enumerator.enumerate();
        workspace.updateNativeState(currentWindows);

        if (++syncCounter >= 15) {
            syncCounter = 0;
            workspace.syncFromWindows(currentWindows, camera);
        }

        const RECT workArea = getPrimaryWorkArea();

        layoutApplier.apply(workspace, camera, workArea, mapper, controller);

        overlay.setSnapshot(&workspace, camera, getPrimaryWorkArea());
        overlay.repaint();

        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}