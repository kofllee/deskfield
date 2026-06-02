#include "OverlayWindow.h"

namespace {
    constexpr wchar_t OverlayClassName[] = L"DeskfieldOverlayWindow";

    int rectWidth(const RECT& rect) {
        return rect.right - rect.left;
    }

    int rectHeight(const RECT& rect) {
        return rect.bottom - rect.top;
    }
}

OverlayWindow::~OverlayWindow() {
    destroy();
}

bool OverlayWindow::create(const RECT& workArea) {
    if (hwnd_ != nullptr) {
        return false;
    }

    workArea_ = workArea;

    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.hInstance = instance;
    wc.lpfnWndProc = OverlayWindow::windowProc;
    wc.lpszClassName = OverlayClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;

    RegisterClassExW(&wc);

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED |
        WS_EX_TRANSPARENT |
        WS_EX_NOACTIVATE |
        WS_EX_TOOLWINDOW |
        WS_EX_TOPMOST,
        OverlayClassName,
        L"Deskfield Overlay",
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

    SetLayeredWindowAttributes(hwnd_, 0, 90, LWA_ALPHA);

    return true;
}

void OverlayWindow::destroy() {
    if (hwnd_ != nullptr) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void OverlayWindow::show() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
    }
}

void OverlayWindow::hide() {
    if (hwnd_ != nullptr) {
        ShowWindow(hwnd_, SW_HIDE);
    }
}

void OverlayWindow::setRenderer(ICanvasRenderer* renderer) {
    renderer_ = renderer;
}

void OverlayWindow::setSnapshot(
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

void OverlayWindow::repaint() {
    if (hwnd_ != nullptr) {
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
}

LRESULT OverlayWindow::windowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
) {
    OverlayWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<OverlayWindow*>(createStruct->lpCreateParams);

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

    self = reinterpret_cast<OverlayWindow*>(
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

LRESULT OverlayWindow::handleMessage(
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

        default:
            return DefWindowProcW(hwnd_, msg, wParam, lParam);
    }
}

void OverlayWindow::paint() {
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