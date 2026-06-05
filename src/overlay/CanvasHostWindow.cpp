#include "CanvasHostWindow.h"

#include <utility>

namespace {
    constexpr wchar_t OverlayClassName[] = L"DeskfieldOverlayWindow";

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

    workArea_ = workArea;

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.hInstance = instance;
    wc.lpfnWndProc = CanvasHostWindow::windowProc;
    wc.lpszClassName = OverlayClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        OverlayClassName,
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

    if (hwnd_ == nullptr) {
        return false;
    }

    return true;
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

void CanvasHostWindow::setRenderer(ICanvasRenderer* renderer) {
    renderer_ = renderer;
}

void CanvasHostWindow::setResizeCallback(std::function<void(const RECT&)> callback) {
    resizeCallback_ = std::move(callback);
}

void CanvasHostWindow::setSnapshot(
    const WorkspaceModel* workspace,
    CanvasCamera camera,
    RECT workArea
) {
    workspace_ = workspace;
    camera_ = camera;
    workArea_ = workArea;

    if (hwnd_ != nullptr) {
        SetWindowPos(
            hwnd_,
            HWND_TOPMOST,
            workArea.left,
            workArea.top,
            rectWidth(workArea),
            rectHeight(workArea),
            SWP_NOACTIVATE | SWP_NOOWNERZORDER
        );
    }
}

void CanvasHostWindow::repaint() {
    if (hwnd_ != nullptr) {
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
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

        case WM_NCDESTROY:
            return DefWindowProcW(hwnd_, msg, wParam, lParam);

        case WM_SIZE: {
            if (resizeCallback_) {
                RECT clientRect{};
                GetClientRect(hwnd_, &clientRect);
                resizeCallback_(clientRect);
            }

            return 0;
        }

        default:
            return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }
}

void CanvasHostWindow::paint() {
    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd_, &ps);

    RECT clientRect{};
    GetClientRect(hwnd_, &clientRect);

    HDC memoryDc = CreateCompatibleDC(hdc);

    HBITMAP bitmap = CreateCompatibleBitmap(
        hdc,
        rectWidth(clientRect),
        rectHeight(clientRect)
    );

    HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

    if (renderer_ != nullptr && workspace_ != nullptr) {
        renderer_->render(
            memoryDc,
            clientRect,
            *workspace_,
            camera_,
            workArea_
        );
    } else {
        HBRUSH background = CreateSolidBrush(RGB(8, 10, 14));
        FillRect(memoryDc, &clientRect, background);
        DeleteObject(background);
    }

    BitBlt(
        hdc,
        0,
        0,
        rectWidth(clientRect),
        rectHeight(clientRect),
        memoryDc,
        0,
        0,
        SRCCOPY
    );

    SelectObject(memoryDc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDc);

    EndPaint(hwnd_, &ps);
}