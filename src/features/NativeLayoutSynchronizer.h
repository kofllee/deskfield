#pragma once

#include "windowing/SourceWindowHost.h"
#include "windowing/WindowController.h"

#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class NativeLayoutSynchronizer {
public:
    void synchronize(
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea,
        const ViewportMapper& mapper,
        const WindowController& controller,
        SourceWindowHost& sourceWindowHost
    );
};