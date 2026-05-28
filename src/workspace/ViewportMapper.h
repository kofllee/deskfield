#pragma once

#include "CanvasTypes.h"

#include <windows.h>

class ViewportMapper {
public:
    RECT mapCanvasToScreen(const CanvasRect& canvasRect, const CanvasCamera& camera, const RECT& workArea) const;
};