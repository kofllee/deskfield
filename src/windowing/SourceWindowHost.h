#pragma once

#include "windowing/WindowController.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class SourceWindowHost {
public:
    bool canApplyNativeLayout(const CanvasWindow& window) const;

    bool applyNativeLayout(
        const CanvasWindow& window,
        const CanvasCamera& camera,
        const RECT& workArea,
        const ViewportMapper& mapper,
        const WindowController& controller
    );

private:
    bool shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const;
    void rememberAppliedRect(HWND hwnd, const RECT& rect);

private:
    HWND lastHwnd_{nullptr};
    RECT lastRect_{};
};

