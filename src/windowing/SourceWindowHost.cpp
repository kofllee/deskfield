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

void SourceWindowHost::forget(HWND hwnd) {
    lastAppliedRects_.erase(hwnd);
}

void SourceWindowHost::clear() {
    lastAppliedRects_.clear();
}

bool SourceWindowHost::shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const {
    const auto it = lastAppliedRects_.find(hwnd);

    if (it == lastAppliedRects_.end()) {
        return false;
    }

    return sameRect(it->second, rect);
}

void SourceWindowHost::rememberAppliedRect(HWND hwnd, const RECT& rect) {
    lastAppliedRects_[hwnd] = rect;
}