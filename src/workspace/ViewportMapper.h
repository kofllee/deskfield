#pragma once

#include "CanvasTypes.h"

#include <windows.h>

class ViewportMapper {
public:
    RECT mapCanvasToNativeRect(
        const CanvasRect& canvasRect,
        const CanvasCamera& camera,
        const RECT& workArea
    ) const;

    RECT mapCanvasToVisualRect(
        const CanvasRect& canvasRect,
        const CanvasCamera& camera,
        const RECT& workArea
    ) const;
};