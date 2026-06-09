#include "CanvasHostWindow.h"

#include <utility>
#include <windowsx.h>

namespace {
    constexpr wchar_t CanvasHostClassName[] = L"DeskfieldCanvasHostWindow";

    int rectWidth(const RECT& rect) {
        return rect.right - rect.left;
    }

    int rectHeight(const RECT& rect) {
        return rect.bottom - rect.top;
    }
}

CanvasHostWindow::~CanvasHostWindow() {
    destroy();
}

bool CanvasHostWindow::create(const RECT& workArea) {
    if (hwnd_ != nullptr) {
        return false;
    }

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.hInstance = instance;
    wc.lpfnWndProc = CanvasHostWindow::windowProc;
    wc.lpszClassName = CanvasHostClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        CanvasHostClassName,
        L"Deskfield",
        WS_POPUP,
        workArea.left,
        workArea.top,
        rectWidth(workArea),
        rectHeight(workArea),
        nullptr,
        nullptr,
        instance,
        this
    );

    return hwnd_ != nullptr;
}

void CanvasHostWindow::destroy() {
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void CanvasHostWindow::show() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);
    }
}

void CanvasHostWindow::hide() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_HIDE);
    }
}

void CanvasHostWindow::setResizeCallback(std::function<void(const RECT&)> callback) {
    resizeCallback_ = std::move(callback);
}

LRESULT CanvasHostWindow::windowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
) {
    CanvasHostWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<CanvasHostWindow*>(createStruct->lpCreateParams);

        if (self == nullptr) {
            return FALSE;
        }

        self->hwnd_ = hwnd;

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(self)
        );

        return TRUE;
    }

    self = reinterpret_cast<CanvasHostWindow*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA)
    );

    if (self != nullptr) {
        if (msg == WM_NCDESTROY) {
            LRESULT result = self->handleMessage(msg, wParam, lParam);
            self->hwnd_ = nullptr;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return result;
        }

        return self->handleMessage(msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CanvasHostWindow::handleMessage(
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (msg) {
        case WM_PAINT:
            paint();
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE: {
            if (resizeCallback_) {
                RECT clientRect{};
                GetClientRect(hwnd_, &clientRect);
                resizeCallback_(clientRect);
            }

            return 0;
        }

        case WM_NCDESTROY:
            return DefWindowProcW(hwnd_, msg, wParam, lParam);

        case WM_LBUTTONDOWN: {
            SetFocus(hwnd_);
            SetCapture(hwnd_);

            POINT point{};
            point.x = GET_X_LPARAM(lParam);
            point.y = GET_Y_LPARAM(lParam);

            if (leftMouseDownCallback_) {
                leftMouseDownCallback_(point);
            }

            return 0;
        }

        case WM_LBUTTONUP:
            if (GetCapture() == hwnd_) {
                ReleaseCapture();
            }
            return 0;

        default:
            return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }
}

void CanvasHostWindow::paint() {
    PAINTSTRUCT ps{};
    BeginPaint(hwnd_, &ps);
    EndPaint(hwnd_, &ps);
}

void CanvasHostWindow::setLeftMouseDownCallback(MouseButtonCallback callback) {
    leftMouseDownCallback_ = std::move(callback);
}