#pragma once

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"
#include "workspace/WindowId.h"

#include <windows.h>

enum class WindowHitZone {
    None,
    Client,
    TitleBar,
    Border
};

struct WindowHitResult {
    WindowId id{};
    WindowHitZone zone{WindowHitZone::None};

    bool hit() const {
        return id.isValid() && zone != WindowHitZone::None;
    }
};

class WindowHitTester {
public:
    WindowHitResult hitTest(
        POINT clientPoint,
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& canvasClientRect,
        const ViewportMapper& mapper
    ) const;

private:
    static bool containsPoint(const RECT& rect, POINT point);
};