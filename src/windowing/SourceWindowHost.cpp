#include "SourceWindowHost.h"

namespace {
    bool sameRect(const RECT& a, const RECT& b) {
        return a.left == b.left &&
               a.top == b.top &&
               a.right == b.right &&
               a.bottom == b.bottom;
    }

    RECT makeParkedRect(const CanvasWindow& window) {
        const int width = static_cast<int>(window.canvasRect.width);
        const int height = static_cast<int>(window.canvasRect.height);

        RECT rect{};
        rect.left = -32000;
        rect.top = -32000;
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;
        return rect;
    }
}

SourcePlacementMode SourceWindowHost::resolvePlacement(const CanvasWindow& window) const {
    if (window.hwnd == nullptr) {
        return SourcePlacementMode::None;
    }

    switch (window.state) {
        case DeskfieldWindowState::NativeInteractive:
            return SourcePlacementMode::InteractiveViewport;

        case DeskfieldWindowState::CanvasFullscreen:
            return SourcePlacementMode::CanvasFullscreen;

        case DeskfieldWindowState::CaptureOnly:
            return SourcePlacementMode::VisibleBackground;

        case DeskfieldWindowState::Normal:
            return SourcePlacementMode::VisibleBackground;

        case DeskfieldWindowState::CanvasMinimized:
            return SourcePlacementMode::VisibleBackground;

        case DeskfieldWindowState::Unavailable:
            return SourcePlacementMode::VisibleBackground;

        case DeskfieldWindowState::Hidden:
        case DeskfieldWindowState::Closed:
            return SourcePlacementMode::None;
    }

    return SourcePlacementMode::None;
}

bool SourceWindowHost::applyPlacement(
    const CanvasWindow& window,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const WindowController& controller
) {
    const SourcePlacementMode placement = resolvePlacement(window);

    switch (placement) {
        case SourcePlacementMode::InteractiveViewport:
        case SourcePlacementMode::CanvasFullscreen:
            return applyInteractiveViewport(
                window,
                camera,
                workArea,
                mapper,
                controller
            );

        case SourcePlacementMode::Parked:
            return parkWindow(window, controller);

        case SourcePlacementMode::VisibleBackground:
        case SourcePlacementMode::HiddenIfSafe:
        case SourcePlacementMode::None:
            return false;
    }

    return false;
}

bool SourceWindowHost::applyInteractiveViewport(
    const CanvasWindow& window,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const WindowController& controller
) {
    if (window.hwnd == nullptr) {
        return false;
    }

    if (window.native.minimized || window.native.maximized) {
        controller.restoreWindow(window.hwnd);
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

bool SourceWindowHost::parkWindow(
    const CanvasWindow& window,
    const WindowController& controller
) {
    if (window.hwnd == nullptr) {
        return false;
    }

    const RECT parkedRect = makeParkedRect(window);

    if (shouldSkipUnchangedRect(window.hwnd, parkedRect)) {
        return false;
    }

    if (!controller.moveWindow(window.hwnd, parkedRect)) {
        return false;
    }

    rememberAppliedRect(window.hwnd, parkedRect);
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