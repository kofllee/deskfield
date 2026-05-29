#pragma once

#include "windowing/WindowController.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class LayoutApplier {
public:
    void apply(const WorkspaceModel& workspace, const CanvasCamera& camera, const RECT& workArea, const ViewportMapper& mapper, const WindowController& controller) const;

private:
    static bool shouldApplyWindow(const ManagedWindow& window);
};