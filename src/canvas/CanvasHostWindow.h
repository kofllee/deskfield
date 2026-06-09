#pragma once

#include <windows.h>

#include <functional>

class CanvasHostWindow {
public:
    using ResizeCallback = std::function<void(const RECT&)>;
    using MouseButtonCallback = std::function<void(POINT)>;

    ~CanvasHostWindow();

    bool create(const RECT& workArea);
    void destroy();

    void show();
    void hide();

    HWND hwnd() const {
        return hwnd_;
    }

    void setResizeCallback(ResizeCallback callback);
    void setLeftMouseDownCallback(MouseButtonCallback callback);

private:
    static LRESULT CALLBACK windowProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );

    LRESULT handleMessage(
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );

    void paint();

private:
    HWND hwnd_{nullptr};

    ResizeCallback resizeCallback_{};
    MouseButtonCallback leftMouseDownCallback_{};
};