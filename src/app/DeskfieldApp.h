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

#include "rendering/D3DCanvasRenderer.h"

#include "capture/GraphicsCaptureManager.h"

#include "input/InputRouter.h"
#include "input/WindowHitTester.h"

#include "DeskfieldMode.h"

#include <windows.h>

#include <chrono>

class DeskfieldApp {
public:
    bool initialize();
    int run();

private:
    void tick(double deltaSeconds);

    void processKeyboardShortcuts(double deltaSeconds);
    void processMessages(bool& shouldQuit);

    void syncWindows();
    void applyWindowStateChanges();

    void applySourcePlacement();
    void renderCanvas();

    void handleCanvasLeftMouseDown(POINT clientPoint);
    void handleCanvasLeftMouseUp(POINT clientPoint);
    void handleCanvasLeftMouseDoubleClick(POINT clientPoint);

    void handleCanvasMiddleMouseDown(POINT clientPoint);
    void handleCanvasMiddleMouseUp(POINT clientPoint);

    void handleCanvasMouseMove(POINT clientPoint);
    void handleCanvasMouseWheel(POINT clientPoint, int wheelDelta);

    void enterNativeInteraction(WindowId id);
    void leaveNativeInteractionExcept(WindowId id);

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

    CanvasHostWindow canvasHost_{};
    D3DCanvasRenderer d3dCanvasRenderer_{};

    WindowHitTester windowHitTester_{};
    InputRouter inputRouter_{};

    CanvasCameraController cameraController_{};

    CanvasCamera camera_{};
    DeskfieldMode mode_{DeskfieldMode::Canvas};

    int syncCounter_{0};
};