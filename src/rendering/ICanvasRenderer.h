#pragma once

#include "workspace/CanvasTypes.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class ICanvasRenderer {
public:
    virtual ~ICanvasRenderer() = default;

    virtual void render(
        HDC hdc,
        const RECT& clientRect,
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea
        ) = 0;
};