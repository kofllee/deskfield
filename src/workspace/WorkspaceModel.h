#pragma once

#include "CanvasTypes.h"
#include "windowing/WindowInfo.h"

#include <windows.h>
#include <vector>

enum class ManagedWindowState {
    Normal,
    Minimized,
    Maximized,
    Hidden
};

struct ManagedWindow {
    HWND hwnd{};

    std::wstring title{};
    std::wstring className{};
    DWORD processId{};

    CanvasRect canvasRect{};
    CanvasRect savedNormalRect{};

    ManagedWindowState state{ManagedWindowState::Normal};

    bool wasMinimized{};
    bool wasMaximized{};
};

class WorkspaceModel {
public:
    void rebuildFromWindows(const std::vector<WindowInfo>& windows);
    void syncFromWindows(const std::vector<WindowInfo>& windows, const CanvasCamera& camera);
    void updateNativeState(const std::vector<WindowInfo>& windows);

    std::vector<ManagedWindow> &windows();
    const std::vector<ManagedWindow> &windows() const;

private:
    std::vector<ManagedWindow> windows_;

    ManagedWindow* findByHwnd(HWND hwnd);
    const ManagedWindow* findByHwnd(HWND hwnd) const;

    static bool containsWindow(const std::vector<WindowInfo>& windows, HWND hwnd);
    static CanvasRect makeInitialCanvasRect(const WindowInfo& window, const CanvasCamera& camera);
    static ManagedWindow makeManagedWindow(const WindowInfo& window, const CanvasCamera& camera);
};