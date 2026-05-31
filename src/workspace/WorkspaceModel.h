#pragma once

#include "CanvasTypes.h"
#include "windowing/WindowSnapshot.h"

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
    void rebuildFromWindows(const std::vector<WindowSnapshot>& windows);
    void syncFromWindows(const std::vector<WindowSnapshot>& windows, const CanvasCamera& camera);
    void updateNativeState(const std::vector<WindowSnapshot>& windows);

    std::vector<ManagedWindow> &windows();
    const std::vector<ManagedWindow> &windows() const;

private:
    std::vector<ManagedWindow> windows_;

    ManagedWindow* findByHwnd(HWND hwnd);
    const ManagedWindow* findByHwnd(HWND hwnd) const;

    static bool containsWindow(const std::vector<WindowSnapshot>& windows, HWND hwnd);
    static CanvasRect makeInitialCanvasRect(const WindowSnapshot& window, const CanvasCamera& camera);
    static ManagedWindow makeManagedWindow(const WindowSnapshot& window, const CanvasCamera& camera);
};