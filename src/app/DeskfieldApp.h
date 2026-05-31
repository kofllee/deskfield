#pragma once

#include "features/NativeLayoutSynchronizer.h"

#include "overlay/OverlayWindow.h"

#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"

#include "workspace/CanvasCameraController.h"
#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

#include <chrono>

#include "DeskfieldMode.h"

class DeskfieldApp {
public:
    bool initialize();
    int run();

private:
    void tick(double deltaSeconds);
    void processInput(double deltaSeconds);
    void processMessages(bool& shouldQuit);
    void updateMode();
    void syncWindows();
    void renderOverlay();

    static RECT getPrimaryWorkArea();

private:
    WindowEnumerator enumerator_{};
    WindowController controller_{};
    WorkspaceModel workspace_{};
    ViewportMapper mapper_{};
    NativeLayoutSynchronizer nativeLayoutSynchronizer_{};
    OverlayWindow overlay_{};
    CanvasCameraController cameraController_{};

    CanvasCamera camera_{};
    DeskfieldMode mode_{DeskfieldMode::Interactive};

    int syncCounter_{0};
};


