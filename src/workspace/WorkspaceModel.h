#pragma once

#include "CanvasTypes.h"
#include "windowing/WindowInfo.h"

#include <windows.h>

#include <vector>

struct ManagedWindow {
    HWND hwnd{};
    CanvasRect canvasRect{};
};

class WorkspaceModel {
public:
    void rebuildFromWindows(const std::vector<WindowInfo>& windows);

    std::vector<ManagedWindow> &windows();
    const std::vector<ManagedWindow> &windows() const;

private:
    std::vector<ManagedWindow> windows_;
};