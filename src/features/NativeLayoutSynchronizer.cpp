#include "NativeLayoutSynchronizer.h"

namespace {
    bool sameRect(const RECT& a, const RECT& b) {
        return a.left == b.left &&
               a.top == b.top &&
               a.right == b.right &&
               a.bottom == b.bottom;
    }
}


bool NativeLayoutSynchronizer::shouldSynchronizeWindow(const CanvasWindow& window) const {
    if (window.hwnd == nullptr) {
        return false;
    }

    if (window.state != DeskfieldWindowState::Normal) {
        return false;
    }

    return true;
}

bool NativeLayoutSynchronizer::shouldSkipUnchangedRect(HWND hwnd, const RECT& rect) const {
    return hwnd == lastHwnd_ && sameRect(rect, lastRect_);
}

void NativeLayoutSynchronizer::synchronize(
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const WindowController& controller
) {
    for (const CanvasWindow& window : workspace.windows()) {
        if (!shouldSynchronizeWindow(window)) {
            continue;
        }

        const RECT nativeRect = mapper.mapCanvasToNativeRect(
            window.canvasRect,
            camera,
            workArea
        );

        if (shouldSkipUnchangedRect(window.hwnd, nativeRect)) {
            continue;
        }

        if (controller.moveWindow(window.hwnd, nativeRect)) {
            lastHwnd_ = window.hwnd;
            lastRect_ = nativeRect;
        }
    }
}
