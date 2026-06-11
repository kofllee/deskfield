#pragma once

#include <windows.h>

#include <functional>

class CanvasHostWindow {
public:
    using ResizeCallback = std::function<void(const RECT&)>;
    using MouseButtonCallback = std::function<void(POINT)>;
    using MouseMoveCallback = std::function<void(POINT)>;
    using MouseWheelCallback = std::function<void(POINT, int)>;

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
    void setLeftMouseUpCallback(MouseButtonCallback callback);
    void setLeftMouseDoubleClickCallback(MouseButtonCallback callback);

    void setMiddleMouseDownCallback(MouseButtonCallback callback);
    void setMiddleMouseUpCallback(MouseButtonCallback callback);

    void setMouseMoveCallback(MouseMoveCallback callback);
    void setMouseWheelCallback(MouseWheelCallback callback);

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
    MouseButtonCallback leftMouseUpCallback_{};
    MouseButtonCallback leftMouseDoubleClickCallback_{};

    MouseButtonCallback middleMouseDownCallback_{};
    MouseButtonCallback middleMouseUpCallback_{};

    MouseMoveCallback mouseMoveCallback_{};
    MouseWheelCallback mouseWheelCallback_{};
};