#include "InputRouter.h"

#include <algorithm>

namespace {
    constexpr double MinZoom = 0.2;
    constexpr double MaxZoom = 2.0;

    double wheelZoomFactor(int wheelDelta) {
        if (wheelDelta > 0) {
            return 1.10;
        }

        if (wheelDelta < 0) {
            return 1.0 / 1.10;
        }

        return 1.0;
    }
}

void InputRouter::onLeftMouseDown(
    POINT point,
    WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& clientRect,
    const ViewportMapper& mapper,
    const WindowHitTester& hitTester
) {
    resetPointer();

    const WindowHitResult hit = hitTester.hitTest(
        point,
        workspace,
        camera,
        clientRect,
        mapper
    );

    if (!hit.hit()) {
        workspace.clearSelection();
        clearNativeInteractionRequested_ = true;
        return;
    }

    workspace.selectWindow(hit.id);
    clearNativeInteractionRequested_ = true;

    CanvasWindow* window = workspace.findById(hit.id);

    if (window == nullptr) {
        return;
    }

    pointer_.startClientPoint = point;
    pointer_.lastClientPoint = point;
    pointer_.activeWindowId = hit.id;
    pointer_.activeZone = hit.zone;
    pointer_.startCanvasRect = window->canvasRect;
    pointer_.startCamera = camera;

    if (hit.zone == WindowHitZone::TitleBar) {
        pointer_.mode = PointerInteractionMode::MovingWindow;
    }
}

void InputRouter::onLeftMouseUp() {
    if (pointer_.mode == PointerInteractionMode::MovingWindow) {
        resetPointer();
    }
}

void InputRouter::onMiddleMouseDown(
    POINT point,
    const CanvasCamera& camera
) {
    resetPointer();

    pointer_.mode = PointerInteractionMode::PanningCanvas;
    pointer_.startClientPoint = point;
    pointer_.lastClientPoint = point;
    pointer_.startCamera = camera;
}

void InputRouter::onMiddleMouseUp() {
    if (pointer_.mode == PointerInteractionMode::PanningCanvas) {
        resetPointer();
    }
}

void InputRouter::onMouseMove(
    POINT point,
    WorkspaceModel& workspace,
    CanvasCamera& camera
) {
    switch (pointer_.mode) {
        case PointerInteractionMode::MovingWindow: {
            if (!pointer_.activeWindowId.isValid()) {
                return;
            }

            const double zoom = std::max(camera.zoom, 0.001);

            const double dx =
                static_cast<double>(point.x - pointer_.startClientPoint.x) / zoom;

            const double dy =
                static_cast<double>(point.y - pointer_.startClientPoint.y) / zoom;

            CanvasRect rect = pointer_.startCanvasRect;
            rect.x += dx;
            rect.y += dy;

            workspace.setCanvasRect(pointer_.activeWindowId, rect);
            pointer_.lastClientPoint = point;
            return;
        }

        case PointerInteractionMode::PanningCanvas: {
            const double zoom = std::max(pointer_.startCamera.zoom, 0.001);

            const double dx =
                static_cast<double>(point.x - pointer_.startClientPoint.x) / zoom;

            const double dy =
                static_cast<double>(point.y - pointer_.startClientPoint.y) / zoom;

            camera.targetX = pointer_.startCamera.targetX - dx;
            camera.targetY = pointer_.startCamera.targetY - dy;

            pointer_.lastClientPoint = point;
            return;
        }

        case PointerInteractionMode::None:
            return;
    }
}

void InputRouter::onWheel(
    POINT point,
    int wheelDelta,
    bool ctrlDown,
    CanvasCamera& camera
) {
    if (!ctrlDown) {
        return;
    }

    const double oldZoom = std::clamp(camera.targetZoom, MinZoom, MaxZoom);
    const double factor = wheelZoomFactor(wheelDelta);
    const double newZoom = std::clamp(oldZoom * factor, MinZoom, MaxZoom);

    if (oldZoom == newZoom) {
        return;
    }

    const double canvasX =
        camera.targetX + static_cast<double>(point.x) / oldZoom;

    const double canvasY =
        camera.targetY + static_cast<double>(point.y) / oldZoom;

    camera.targetZoom = newZoom;
    camera.targetX = canvasX - static_cast<double>(point.x) / newZoom;
    camera.targetY = canvasY - static_cast<double>(point.y) / newZoom;
}

bool InputRouter::consumeClearNativeInteractionRequest() {
    const bool requested = clearNativeInteractionRequested_;
    clearNativeInteractionRequested_ = false;
    return requested;
}

void InputRouter::resetPointer() {
    pointer_ = {};
}