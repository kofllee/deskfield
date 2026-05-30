#include "CanvasCameraController.h"

#include <algorithm>

void CanvasCameraController::resetTargets(CanvasCamera &camera) const {
    camera.targetX = camera.x;
    camera.targetY = camera.y;
    camera.targetZoom = camera.zoom;
}

void CanvasCameraController::pan(CanvasCamera &camera, double dx, double dy) const {
    camera.targetX += dx / camera.zoom;
    camera.targetY += dy / camera.zoom;
}

void CanvasCameraController::zoom(CanvasCamera &camera, double zoomFactor) const {
    camera.targetZoom = std::clamp(camera.targetZoom * zoomFactor, 0.2, 2.0);
}

void CanvasCameraController::animate(CanvasCamera &camera, double deltaSeconds) const {
    const double speed = 14.0;
    const double t = std::clamp(deltaSeconds * speed, 0.0, 1.0);

    camera.x = lerp(camera.x, camera.targetX, t);
    camera.y = lerp(camera.y, camera.targetY, t);
    camera.zoom = lerp(camera.zoom, camera.targetZoom, t);
}

void CanvasCameraController::focusRect(CanvasCamera &camera, const CanvasRect &rect, double viewportWidth, double viewportHeight) const {
    camera.targetZoom = 1.0;
    camera.targetX = rect.x + rect.width * 0.5 - viewportWidth * 0.5;
    camera.targetY = rect.y + rect.height * 0.5 - viewportHeight * 0.5;
}

double CanvasCameraController::lerp(double a, double b, double t) {
    return a + (b - a) * t;
}
