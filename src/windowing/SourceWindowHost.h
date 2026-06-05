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

    void forget(HWND hwnd);
    void clear();

private:
    bool shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const;
    void rememberAppliedRect(HWND hwnd, const RECT& rect);

private:
    std::unordered_map<HWND, RECT> lastAppliedRects_{};
};

