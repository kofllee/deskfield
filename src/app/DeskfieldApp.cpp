#include "DeskfieldApp.h"

#include <iostream>
#include <thread>

bool DeskfieldApp::initialize() {
    SetProcessDPIAware();

    const RECT workArea = getPrimaryWorkArea();

    if (!overlay_.create(workArea)) {
        std::wcout << L"Failed to create overlay. Error: " << GetLastError() << L"\n";
        return false;
    }

    overlay_.setRenderer(&debugCanvasRenderer_);
    overlay_.show();

    camera_.x = 0.0;
    camera_.y = 0.0;
    camera_.zoom = 1.0;

    cameraController_.resetTargets(camera_);

    auto snapshots = enumerator_.enumerate();
    windowStateTracker_.update(std::move(snapshots));

    workspace_.clear();
    registerInitialWindows();

    return true;
}

int DeskfieldApp::run() {
    auto lastTime = std::chrono::steady_clock::now();

    bool shouldQuit = false;

    while (!shouldQuit) {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> delta = now - lastTime;
        lastTime = now;

        tick(delta.count());
        processMessages(shouldQuit);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}

void DeskfieldApp::tick(double deltaSeconds) {
    processInput(deltaSeconds);

    cameraController_.animate(camera_, deltaSeconds);

    updateMode();
    syncWindows();

    graphicsCaptureManager_.update();

    const RECT workArea = getPrimaryWorkArea();

    if (mode_ == DeskfieldMode::Interactive) {
        nativeLayoutSynchronizer_.synchronize(
            workspace_,
            camera_,
            workArea,
            mapper_,
            controller_,
            sourceWindowHost_
        );
    }

    renderOverlay();
}

void DeskfieldApp::processInput(double) {
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        PostQuitMessage(0);
        return;
    }

    constexpr double panSpeed = 28.0;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        cameraController_.pan(camera_, -panSpeed, 0.0);
    }

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        cameraController_.pan(camera_, panSpeed, 0.0);
    }

    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        cameraController_.pan(camera_, 0.0, -panSpeed);
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        cameraController_.pan(camera_, 0.0, panSpeed);
    }

    if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) {
        cameraController_.zoom(camera_, 0.98);
    }

    if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) {
        cameraController_.zoom(camera_, 1.02);
    }
}

void DeskfieldApp::updateMode() {
    constexpr double overviewThreshold = 0.65;

    mode_ = camera_.zoom < overviewThreshold
        ? DeskfieldMode::Overview
        : DeskfieldMode::Interactive;
}

void DeskfieldApp::syncWindows() {
    auto snapshots = enumerator_.enumerate();
    windowStateTracker_.update(std::move(snapshots));

    applyWindowStateChanges();
}

void DeskfieldApp::applyWindowStateChanges() {
    for (const WindowSnapshot& snapshot : windowStateTracker_.removedWindows()) {
        const WindowId id = windowRegistry_.findByHwnd(snapshot.hwnd);

        if (!id.isValid()) {
            continue;
        }

        graphicsCaptureManager_.detach(id);
        workspace_.removeWindow(id);
        windowRegistry_.remove(snapshot.hwnd);
    }

    for (const WindowSnapshot& snapshot : windowStateTracker_.addedWindows()) {
        const WindowId id = windowRegistry_.getOrCreate(snapshot.hwnd);

        workspace_.addWindow(snapshot, id, camera_);
        graphicsCaptureManager_.attach(id, snapshot.hwnd);
    }

    for (const WindowSnapshot& snapshot : windowStateTracker_.updatedWindows()) {
        const WindowId id = windowRegistry_.findByHwnd(snapshot.hwnd);

        if (!id.isValid()) {
            continue;
        }

        workspace_.updateNativeState(id, snapshot);
    }

    for (const WindowSnapshot& snapshot : windowStateTracker_.current()) {
        const WindowId id = windowRegistry_.findByHwnd(snapshot.hwnd);

        if (!id.isValid()) {
            continue;
        }

        workspace_.updateMetadata(id, snapshot);
    }
}

void DeskfieldApp::renderOverlay() {
    const RECT workArea = getPrimaryWorkArea();

    overlay_.setSnapshot(
        &workspace_,
        camera_,
        workArea
    );

    overlay_.repaint();
}

void DeskfieldApp::registerInitialWindows() {
    for (const WindowSnapshot& snapshot : windowStateTracker_.current()) {
        const WindowId id = windowRegistry_.getOrCreate(snapshot.hwnd);

        workspace_.addWindow(snapshot, id, camera_);
        graphicsCaptureManager_.attach(id, snapshot.hwnd);
    }
}

void DeskfieldApp::processMessages(bool& shouldQuit) {
    MSG msg{};

    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            shouldQuit = true;
            return;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

RECT DeskfieldApp::getPrimaryWorkArea() {
    RECT workArea{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    return workArea;
}