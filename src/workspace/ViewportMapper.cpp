#include "ViewportMapper.h"

RECT ViewportMapper::mapCanvasToScreen(const CanvasRect &canvasRect, const CanvasCamera &camera, const RECT &workArea) const {
    RECT result{};

    const double screenX = static_cast<double>(workArea.left) + (canvasRect.x - camera.x) * camera.zoom;
    const double screenY = static_cast<double>(workArea.top) + (canvasRect.y - camera.y) * camera.zoom;

    const double screenWidth = canvasRect.width * camera.zoom;
    const double screenHeight = canvasRect.height * camera.zoom;

    result.left = static_cast<LONG>(screenX);
    result.top = static_cast<LONG>(screenY);
    result.right = static_cast<LONG>(screenX + screenWidth);
    result.bottom = static_cast<LONG>(screenY + screenHeight);

    return result;
}
