#pragma once

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

    void setResizeCallback(std::function<void(const RECT&)> callback);

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

    std::function<void(const RECT&)> resizeCallback_{};
};
