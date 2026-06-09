#include "WindowHitTester.h"

#include <algorithm>

namespace {
    constexpr int TitleBarHeight = 30;
    constexpr int BorderThickness = 6;

    bool isRenderableWindowState(DeskfieldWindowState state) {
        return state != DeskfieldWindowState::Hidden &&
               state != DeskfieldWindowState::Closed;
    }
}

WindowHitResult WindowHitTester::hitTest(
    POINT clientPoint,
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& canvasClientRect,
    const ViewportMapper& mapper
) const {
    const auto& windows = workspace.windows();

    for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
        const CanvasWindow& window = *it;

        if (!isRenderableWindowState(window.state)) {
            continue;
        }

        RECT visualRect = mapper.mapCanvasToVisualRect(
            window.canvasRect,
            camera,
            canvasClientRect
        );

        const int scaledTitleHeight = std::max(
            18,
            static_cast<int>(static_cast<double>(TitleBarHeight) * camera.zoom)
        );

        visualRect.top -= scaledTitleHeight;

        if (!containsPoint(visualRect, clientPoint)) {
            continue;
        }

        RECT titleRect = visualRect;
        titleRect.bottom = std::max(titleRect.top + scaledTitleHeight, visualRect.bottom);

        if (containsPoint(titleRect, clientPoint)) {
            return WindowHitResult{
                window.id,
                WindowHitZone::TitleBar
            };
        }

        const bool onBorder =
            clientPoint.x < visualRect.left + BorderThickness ||
            clientPoint.x >= visualRect.right - BorderThickness ||
            clientPoint.y < visualRect.top + BorderThickness ||
            clientPoint.y >= visualRect.bottom - BorderThickness;

        return WindowHitResult{
            window.id,
            onBorder ? WindowHitZone::Border : WindowHitZone::Client
        };
    }

    return {};
}

bool WindowHitTester::containsPoint(const RECT& rect, POINT point) {
    return point.x >= rect.left &&
           point.x < rect.right &&
           point.y >= rect.top &&
           point.y < rect.bottom;
}