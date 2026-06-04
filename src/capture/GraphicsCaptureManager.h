#pragma once

#include "capture/CapturedWindow.h"

#include "workspace/WindowId.h"

#include <windows.h>

#include <unordered_map>

class GraphicsCaptureManager {
public:
    bool attach(WindowId id, HWND hwnd);
    void detach(WindowId id);
    void detachAll();

    void update();

    bool isAttached(WindowId id) const;
    const CapturedWindow* find(WindowId id) const;

private:
    std::unordered_map<WindowId, CapturedWindow> capturedWindows_{};
};
