#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"
#include "features/LayoutApplier.h"
#include "overlay/OverlayWindow.h"
#include "workspace/CanvasCameraController.h"
#include "app/DeskfieldMode.h"

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
    CanvasCameraController cameraController;

    DeskfieldMode mode = DeskfieldMode::Interactive;

    if (!overlay.create(getPrimaryWorkArea())) {
        std::wcout << L"Failed to create overlay. Error: " << GetLastError() << L"\n";
        return 1;
    }

    overlay.show();

    CanvasCamera camera{};
    camera.x = 0.0;
    camera.y = 0.0;
    camera.zoom = 1.0;
    cameraController.resetTargets(camera);

    const auto windows = enumerator.enumerate();
    workspace.rebuildFromWindows(windows);

    auto lastTime = std::chrono::steady_clock::now();

    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> delta = now - lastTime;
        lastTime = now;

        const double deltaSeconds = delta.count();

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        constexpr double panSpeed = 28.0;

        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            cameraController.pan(camera, -panSpeed, 0.0);
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            cameraController.pan(camera, panSpeed, 0.0);
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            cameraController.pan(camera, 0.0, -panSpeed);
        }

        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            cameraController.pan(camera, 0.0, panSpeed);
        }

        if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) {
            cameraController.zoom(camera, 0.98);
        }

        if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) {
            cameraController.zoom(camera, 1.02);
        }

        cameraController.animate(camera, deltaSeconds);

        constexpr double overviewThreshold = 0.65;
        mode = camera.zoom < overviewThreshold
            ? DeskfieldMode::Overview
            : DeskfieldMode::Interactive;

        static int syncCounter = 0;

        const auto currentWindows = enumerator.enumerate();
        workspace.updateNativeState(currentWindows);

        if (++syncCounter >= 15) {
            syncCounter = 0;
            workspace.syncFromWindows(currentWindows, camera);
        }

        const RECT workArea = getPrimaryWorkArea();

        if (mode == DeskfieldMode::Interactive) {
            layoutApplier.apply(workspace, camera, workArea, mapper, controller);
        }

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