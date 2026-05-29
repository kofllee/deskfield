#include "OverlayWindow.h"

#include <string>

namespace {
    constexpr wchar_t OverlayClassName[] = L"DeskfieldOverlayWindow";

    int rectWidth(const RECT& rect) {
        return rect.right - rect.left;
    }

    int rectHeight(const RECT& rect) {
        return rect.bottom - rect.top;
    }

    RECT canvasToScreen(const CanvasRect& rect, const CanvasCamera& camera, const RECT& workArea) {
        const double zoom = camera.zoom;

        RECT out{};
        out.left = static_cast<LONG>(workArea.left + (rect.x - camera.x) * zoom);
        out.top = static_cast<LONG>(workArea.top + (rect.y - camera.y) * zoom);
        out.right = static_cast<LONG>(out.left + rect.width * zoom);
        out.bottom = static_cast<LONG>(out.top + rect.height * zoom);

        return out;
    }

    bool intersects(const RECT& a, const RECT& b) {
        RECT tmp{};
        return IntersectRect(&tmp, &a, &b) != FALSE;
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
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
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

void OverlayWindow::setSnapshot(const WorkspaceModel *workspace, CanvasCamera camera, RECT workArea) {
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

LRESULT OverlayWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

    HBRUSH background = CreateSolidBrush(RGB(8, 10, 14));
    FillRect(memoryDc, &clientRect, background);
    DeleteObject(background);

    drawGrid(memoryDc, clientRect);
    drawWindow(memoryDc);

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

void OverlayWindow::drawGrid(HDC hdc, const RECT& clientRect) {
    const int baseStep = 120;
    const int step = static_cast<int>(baseStep * camera_.zoom);

    if (step < 24) {
        return;
    }

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(45, 50, 60));
    HGDIOBJ oldPen = SelectObject(hdc, gridPen);

    const int offsetX = static_cast<int>(-camera_.x * camera_.zoom) % step;
    const int offsetY = static_cast<int>(-camera_.y * camera_.zoom) % step;

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

void OverlayWindow::drawWindow(HDC hdc) {
    if (workspace_ == nullptr) {
        return;
    }

    RECT visibleArea{};
    GetClientRect(hwnd_, &visibleArea);

    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(120, 170, 255));
    HBRUSH cardBrush = CreateSolidBrush(RGB(24, 30, 42));
    HBRUSH titleBrush = CreateSolidBrush(RGB(18, 24, 34));

    HGDIOBJ oldPen = SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = SelectObject(hdc, cardBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(220, 230, 245));

    for (const auto& window: workspace_->windows()) {
        const RECT screenRect = canvasToScreen(window.canvasRect, camera_, workArea_);

        RECT localRect = screenRect;
        OffsetRect(&localRect, -workArea_.left, -workArea_.top);

        if (!intersects(localRect, visibleArea)) {
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

        RECT titleTextRect = titleBackground;
        titleTextRect.left += titlePaddingX;
        titleTextRect.right -= titlePaddingX;
        titleTextRect.top += 4;

        DrawTextW(
            hdc,
            window.title.c_str(),
            static_cast<int>(window.title.size()),
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
