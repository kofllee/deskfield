#pragma once

#include "features/NativeLayoutSynchronizer.h"

#include "canvas/CanvasHostWindow.h"

#include "windowing/WindowController.h"
#include "windowing/WindowEnumerator.h"
#include "windowing/WindowStateTracker.h"

#include "workspace/CanvasCameraController.h"
#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"
#include "workspace/WindowRegistry.h"

#include "rendering/GdiDebugCanvasRenderer.h"
#include "rendering/D3DCanvasRenderer.h"

#include "capture/GraphicsCaptureManager.h"

#include "DeskfieldMode.h"

#include <windows.h>

#include <chrono>


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
    void applyWindowStateChanges();
    void renderCanvas();

    void registerInitialWindows();

    static RECT getPrimaryWorkArea();

private:
    WindowEnumerator enumerator_{};
    WindowStateTracker windowStateTracker_{};

    WindowController controller_{};
    SourceWindowHost sourceWindowHost_{};

    WorkspaceModel workspace_{};
    WindowRegistry windowRegistry_{};
    ViewportMapper mapper_{};

    GraphicsCaptureManager graphicsCaptureManager_{};
    NativeLayoutSynchronizer nativeLayoutSynchronizer_{};

    GdiDebugCanvasRenderer debugCanvasRenderer_{};
    CanvasHostWindow canvasHost_{};
    D3DCanvasRenderer d3dCanvasRenderer_{};

    CanvasCameraController cameraController_{};

    CanvasCamera camera_{};
    DeskfieldMode mode_{DeskfieldMode::Interactive};

    int syncCounter_{0};
};


