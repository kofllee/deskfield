#pragma once

#include "windowing/WindowController.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

#include <unordered_map>

enum class SourcePlacementMode {
    None,
    VisibleBackground,
    InteractiveViewport,
    Parked,
    HiddenIfSafe,
    CanvasFullscreen
};

class SourceWindowHost {
public:
    SourcePlacementMode resolvePlacement(const CanvasWindow& window) const;

    bool applyPlacement(
        const CanvasWindow& window,
        const CanvasCamera& camera,
        const RECT& workArea,
        const ViewportMapper& mapper,
        const WindowController& controller
    );

    void forget(HWND hwnd);
    void clear();

private:
    bool applyInteractiveViewport(
        const CanvasWindow& window,
        const CanvasCamera& camera,
        const RECT& workArea,
        const ViewportMapper& mapper,
        const WindowController& controller
    );

    bool parkWindow(
        const CanvasWindow& window,
        const WindowController& controller
    );

    bool shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const;
    void rememberAppliedRect(HWND hwnd, const RECT& rect);

private:
    std::unordered_map<HWND, RECT> lastAppliedRects_{};
};