#pragma once

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
        const WindowController& controller);

private:
    bool shouldSynchronizeWindow(const CanvasWindow& window) const;
    bool shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const;

private:
    HWND lastHwnd_{nullptr};
    RECT lastRect_{};
};