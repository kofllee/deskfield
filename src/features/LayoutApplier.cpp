#include "LayoutApplier.h"

bool LayoutApplier::shouldApplyWindow(const ManagedWindow& window) {
    if (window.hwnd == nullptr || !IsWindow(window.hwnd)) {
        return false;
    }

    if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) {
        return false;
    }

    if (window.state != ManagedWindowState::Normal) {
        return false;
    }

    return true;
}

void LayoutApplier::apply(const WorkspaceModel &workspace, const CanvasCamera &camera, const RECT &workArea, const ViewportMapper &mapper, const WindowController &controller) const {
    for (const auto& window : workspace.windows()) {
        if (!shouldApplyWindow(window)) {
            continue;
        }

        const RECT screenRect = mapper.mapCanvasToScreen(
            window.canvasRect,
            camera,
            workArea
        );

        controller.moveWindow(window.hwnd, screenRect);
    }
}

