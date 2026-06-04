#include "SourceWindowHost.h"

namespace {
    bool sameRect(const RECT& a, const RECT& b) {
        return a.left == b.left &&
               a.top == b.top &&
               a.right == b.right &&
               a.bottom == b.bottom;
    }
}

bool SourceWindowHost::canApplyNativeLayout(const CanvasWindow& window) const {
    if (window.hwnd == nullptr) {
        return false;
    }

    return window.state == DeskfieldWindowState::Normal;
}

bool SourceWindowHost::applyNativeLayout(
    const CanvasWindow& window,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const WindowController& controller
) {
    if (!canApplyNativeLayout(window)) {
        return false;
    }

    const RECT nativeRect = mapper.mapCanvasToNativeRect(
        window.canvasRect,
        camera,
        workArea
    );

    if (shouldSkipUnchangedRect(window.hwnd, nativeRect)) {
        return false;
    }

    if (!controller.moveWindow(window.hwnd, nativeRect)) {
        return false;
    }

    rememberAppliedRect(window.hwnd, nativeRect);
    return true;
}

bool SourceWindowHost::shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const {
    return hwnd == lastHwnd_ && sameRect(rect, lastRect_);
}

void SourceWindowHost::rememberAppliedRect(HWND hwnd, const RECT& rect) {
    lastHwnd_ = hwnd;
    lastRect_ = rect;
}
