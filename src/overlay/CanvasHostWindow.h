#pragma once

#include "rendering/ICanvasRenderer.h"

#include "workspace/CanvasTypes.h"
#include "workspace/WorkspaceModel.h"

#include <functional>

#include <windows.h>

class CanvasHostWindow {
public:
    CanvasHostWindow() = default;
    ~CanvasHostWindow();

    CanvasHostWindow(const CanvasHostWindow&) = delete;
    CanvasHostWindow& operator=(const CanvasHostWindow&) = delete;

    bool create(const RECT& workArea);
    void destroy();

    void show();
    void hide();

    void setRenderer(ICanvasRenderer* renderer);
    void setResizeCallback(std::function<void(const RECT&)> callback);

    void setSnapshot(
        const WorkspaceModel* workspace,
        CanvasCamera camera,
        RECT workArea
    );

    void repaint();

    HWND hwnd() const {
        return hwnd_;
    }

private:
    static LRESULT CALLBACK windowProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );

    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void paint();

private:
    HWND hwnd_{nullptr};

    ICanvasRenderer* renderer_{nullptr};
    std::function<void(const RECT&)> resizeCallback_{};

    const WorkspaceModel* workspace_{nullptr};
    CanvasCamera camera_{};
    RECT workArea_{};
};
