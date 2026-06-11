#pragma once

#include "input/WindowHitTester.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WindowId.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

enum class PointerInteractionMode {
    None,
    PanningCanvas,
    MovingWindow
};

struct PointerState {
    PointerInteractionMode mode{PointerInteractionMode::None};

    POINT startClientPoint{};
    POINT lastClientPoint{};

    WindowId activeWindowId{};
    WindowHitZone activeZone{WindowHitZone::None};

    CanvasRect startCanvasRect{};
    CanvasCamera startCamera{};
};

class InputRouter {
public:
    void onLeftMouseDown(
        POINT point,
        WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& clientRect,
        const ViewportMapper& mapper,
        const WindowHitTester& hitTester
    );

    void onLeftMouseUp();

    void onMiddleMouseDown(
        POINT point,
        const CanvasCamera& camera
    );

    void onMiddleMouseUp();

    void onMouseMove(
        POINT point,
        WorkspaceModel& workspace,
        CanvasCamera& camera
    );

    void onWheel(
        POINT point,
        int wheelDelta,
        bool ctrlDown,
        CanvasCamera& camera
    );

    bool consumeClearNativeInteractionRequest();

private:
    void resetPointer();

private:
    PointerState pointer_{};
    bool clearNativeInteractionRequested_{};
};