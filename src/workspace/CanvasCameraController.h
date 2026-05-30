#pragma once

#include "workspace/CanvasTypes.h"

class CanvasCameraController {
public:
    void resetTargets(CanvasCamera& camera) const;

    void pan(CanvasCamera& camera, double dx, double dy) const;
    void zoom(CanvasCamera& camera, double zoomFactor) const;
    void animate(CanvasCamera& camera, double deltaSeconds) const;

    void focusRect(CanvasCamera& camera, const CanvasRect& rect, double viewportWidth, double viewportHeight) const;

private:
    static double lerp(double a, double b, double t);
};

