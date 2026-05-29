#pragma once

#include "workspace/CanvasTypes.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class OverlayWindow {
public:
    OverlayWindow() = default;
    ~OverlayWindow();

    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;

    bool create(const RECT& workArea);
    void destroy();

    void show();
    void hide();

    void setSnapshot(const WorkspaceModel* workspace, CanvasCamera camera, RECT workArea);

    void repaint();

    HWND hwnd() const {
        return hwnd_;
    }
private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void paint();
    void drawGrid(HDC hdc, const RECT& clientRect);
    void drawWindow(HDC hdc);

private:
    HWND hwnd_ = nullptr;

    const WorkspaceModel* workspace_ = nullptr;
    CanvasCamera camera_{};
    RECT workArea_{};
};

