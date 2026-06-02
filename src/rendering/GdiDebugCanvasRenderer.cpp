#include "GdiDebugCanvasRenderer.h"

#include <string>

void GdiDebugCanvasRenderer::render(
    HDC hdc,
    const RECT& clientRect,
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea
) {
    drawBackground(hdc, clientRect);
    drawGrid(hdc, clientRect, camera);
    drawWindows(hdc, clientRect, workspace, camera, workArea);
}

void GdiDebugCanvasRenderer::drawBackground(HDC hdc, const RECT& clientRect) {
    HBRUSH background = CreateSolidBrush(RGB(8, 10, 14));
    FillRect(hdc, &clientRect, background);
    DeleteObject(background);
}

void GdiDebugCanvasRenderer::drawGrid(
    HDC hdc,
    const RECT& clientRect,
    const CanvasCamera& camera
) {
    const int baseStep = 120;
    const int step = static_cast<int>(baseStep * camera.zoom);

    if (step < 24) {
        return;
    }

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(45, 50, 60));
    HGDIOBJ oldPen = SelectObject(hdc, gridPen);

    const int offsetX = static_cast<int>(-camera.x * camera.zoom) % step;
    const int offsetY = static_cast<int>(-camera.y * camera.zoom) % step;

    for (int x = offsetX; x < rectWidth(clientRect); x += step) {
        MoveToEx(hdc, x, 0, nullptr);
        LineTo(hdc, x, rectHeight(clientRect));
    }

    for (int y = offsetY; y < rectHeight(clientRect); y += step) {
        MoveToEx(hdc, 0, y, nullptr);
        LineTo(hdc, rectWidth(clientRect), y);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);
}

void GdiDebugCanvasRenderer::drawWindows(
    HDC hdc,
    const RECT& clientRect,
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea
) {
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(120, 170, 255));
    HBRUSH cardBrush = CreateSolidBrush(RGB(24, 30, 42));
    HBRUSH titleBrush = CreateSolidBrush(RGB(18, 24, 34));

    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = SelectObject(hdc, cardBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(220, 230, 245));

    for (const auto& window : workspace.windows()) {
        const RECT localRect = mapCanvasToVisualLocalRect(
            window.canvasRect,
            camera,
            workArea
        );

        if (!intersects(localRect, clientRect)) {
            continue;
        }

        Rectangle(
            hdc,
            localRect.left,
            localRect.top,
            localRect.right,
            localRect.bottom
        );

        constexpr int titleHeight = 24;
        constexpr int titlePaddingX = 8;
        constexpr int titleGap = 4;

        RECT titleBackground{};
        titleBackground.left = localRect.left;
        titleBackground.right = localRect.right;
        titleBackground.bottom = localRect.top - titleGap;
        titleBackground.top = titleBackground.bottom - titleHeight;

        if (titleBackground.top < 0) {
            titleBackground.top = localRect.top + titleGap;
            titleBackground.bottom = titleBackground.top + titleHeight;
        }

        HGDIOBJ oldTitleBrush = SelectObject(hdc, titleBrush);

        Rectangle(
            hdc,
            titleBackground.left,
            titleBackground.top,
            titleBackground.right,
            titleBackground.bottom
        );

        SelectObject(hdc, oldTitleBrush);

        std::wstring title = L"#" + std::to_wstring(window.id.value) + L" " + window.title;

        RECT titleTextRect = titleBackground;
        titleTextRect.left += titlePaddingX;
        titleTextRect.right -= titlePaddingX;
        titleTextRect.top += 4;

        DrawTextW(
            hdc,
            title.c_str(),
            static_cast<int>(title.size()),
            &titleTextRect,
            DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX
        );
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);

    DeleteObject(titleBrush);
    DeleteObject(cardBrush);
    DeleteObject(borderPen);
}

RECT GdiDebugCanvasRenderer::mapCanvasToVisualLocalRect(
    const CanvasRect& rect,
    const CanvasCamera& camera,
    const RECT& workArea
) {
    const double zoom = camera.zoom;

    RECT out{};
    out.left = static_cast<LONG>((rect.x - camera.x) * zoom);
    out.top = static_cast<LONG>((rect.y - camera.y) * zoom);
    out.right = static_cast<LONG>(out.left + rect.width * zoom);
    out.bottom = static_cast<LONG>(out.top + rect.height * zoom);

    return out;
}

bool GdiDebugCanvasRenderer::intersects(const RECT& a, const RECT& b) {
    RECT tmp{};
    return IntersectRect(&tmp, &a, &b) != FALSE;
}

int GdiDebugCanvasRenderer::rectWidth(const RECT& rect) {
    return rect.right - rect.left;
}

int GdiDebugCanvasRenderer::rectHeight(const RECT& rect) {
    return rect.bottom - rect.top;
}

