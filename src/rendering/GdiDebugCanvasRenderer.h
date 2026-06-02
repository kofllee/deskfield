#pragma once

#include "ICanvasRenderer.h"

class GdiDebugCanvasRenderer final: public ICanvasRenderer {
public:
    void render(
        HDC hdc,
        const RECT& clientRect,
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea
        );

private:
    void drawBackground(HDC hdc, const RECT& clientRect);
    void drawGrid(HDC hdc, const RECT& clientRect, const CanvasCamera& camera);
    void drawWindows(
        HDC hdc,
        const RECT& clientRect,
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea
    );

    static RECT mapCanvasToVisualLocalRect(
        const CanvasRect& rect,
        const CanvasCamera& camera,
        const RECT& workArea
    );

    static bool intersects(const RECT& a, const RECT& b);
    static int rectWidth(const RECT& rect);
    static int rectHeight(const RECT& rect);
};