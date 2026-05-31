#include "ViewportMapper.h"

RECT ViewportMapper::mapCanvasToNativeRect(
    const CanvasRect& canvasRect,
    const CanvasCamera& camera,
    const RECT& workArea
) const {
    const double x = static_cast<double>(workArea.left) + (canvasRect.x - camera.x);
    const double y = static_cast<double>(workArea.top) + (canvasRect.y - camera.y);

    RECT result{};
    result.left = static_cast<LONG>(x);
    result.top = static_cast<LONG>(y);
    result.right = static_cast<LONG>(x + canvasRect.width);
    result.bottom = static_cast<LONG>(y + canvasRect.height);

    return result;
}

RECT ViewportMapper::mapCanvasToVisualRect(
    const CanvasRect& canvasRect,
    const CanvasCamera& camera,
    const RECT& workArea
) const {
    const double x = static_cast<double>(workArea.left) + (canvasRect.x - camera.x) * camera.zoom;
    const double y = static_cast<double>(workArea.top) + (canvasRect.y - camera.y) * camera.zoom;

    RECT result{};
    result.left = static_cast<LONG>(x);
    result.top = static_cast<LONG>(y);
    result.right = static_cast<LONG>(x + canvasRect.width * camera.zoom);
    result.bottom = static_cast<LONG>(y + canvasRect.height * camera.zoom);

    return result;
}
