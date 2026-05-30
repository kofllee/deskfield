#pragma once

#include <windows.h>

struct CanvasRect {
    double x{};
    double y{};
    double width{};
    double height{};
};

struct CanvasCamera {
    double x{};
    double y{};
    double zoom{1.0};

    double targetX{};
    double targetY{};
    double targetZoom{1.0};
};

inline CanvasRect rectToCanvasRect(const RECT& rect) {
    return CanvasRect{
        static_cast<double>(rect.left),
        static_cast<double>(rect.top),
        static_cast<double>(rect.right - rect.left),
        static_cast<double>(rect.bottom - rect.top)
    };
}

inline RECT canvasRectToRect(const CanvasRect& canvasRect) {
    RECT result{};
    result.left = static_cast<LONG>(canvasRect.x);
    result.top = static_cast<LONG>(canvasRect.y);
    result.right = static_cast<LONG>(canvasRect.x + canvasRect.width);
    result.bottom = static_cast<LONG>(canvasRect.y + canvasRect.height);
    return result;
}