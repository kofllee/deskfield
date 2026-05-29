#include "WorkspaceModel.h"

#include <algorithm>

void WorkspaceModel::rebuildFromWindows(const std::vector<WindowInfo>& windows) {
    windows_.clear();

    for (const WindowInfo& window : windows) {
        ManagedWindow managed{};
        managed.hwnd = window.hwnd;
        managed.canvasRect = rectToCanvasRect(window.normalRect);

        windows_.push_back(managed);
    }
}

void WorkspaceModel::syncFromWindows(const std::vector<WindowInfo> &windows, const CanvasCamera &camera) {
    windows_.erase(std::remove_if(windows_.begin(), windows_.end(),
        [&](const ManagedWindow& managed) {
            return managed.hwnd == nullptr || !IsWindow(managed.hwnd) || !containsWindow(windows, managed.hwnd);
        }), windows_.end());

    for (const WindowInfo& window : windows) {
        if (findByHwnd(window.hwnd) != nullptr) {
            continue;
        }

        ManagedWindow managed{};
        managed.hwnd = window.hwnd;
        managed.canvasRect = makeInitialCanvasRect(window, camera);

        windows_.push_back(managed);
    }
}

void WorkspaceModel::updateNativeState(const std::vector<WindowInfo> &windows) {
    for (ManagedWindow& managed : windows_) {
        const auto it = std::find_if(windows.begin(), windows.end(), [&](const WindowInfo& window) { return window.hwnd == managed.hwnd; });

        if (it == windows.end()) {
            continue;
        }

        const auto& window = *it;

        if (window.minimized) {
            managed.state = ManagedWindowState::Minimized;
            managed.wasMinimized = true;
            continue;
        }

        if (window.maximized) {
            if (!managed.wasMaximized) {
                managed.savedNormalRect = managed.canvasRect;
            }

            managed.state = ManagedWindowState::Maximized;
            managed.wasMaximized = true;
            continue;
        }

        if (managed.wasMaximized) {
            managed.canvasRect = managed.savedNormalRect;
        }

        managed.state = ManagedWindowState::Normal;
        managed.wasMinimized = false;
        managed.wasMaximized = false;
    }
}

std::vector<ManagedWindow>& WorkspaceModel::windows() { return windows_; }

const std::vector<ManagedWindow>& WorkspaceModel::windows() const { return windows_; }

ManagedWindow* WorkspaceModel::findByHwnd(HWND hwnd) {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hwnd](const ManagedWindow& window) {
            return window.hwnd == hwnd;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

const ManagedWindow* WorkspaceModel::findByHwnd(HWND hwnd) const {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hwnd](const ManagedWindow& window) {
            return window.hwnd == hwnd;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

bool WorkspaceModel::containsWindow(const std::vector<WindowInfo>& windows, HWND hwnd) {
    return std::any_of(windows.begin(), windows.end(), [hwnd](const WindowInfo& window) { return window.hwnd == hwnd; });
}

CanvasRect WorkspaceModel::makeInitialCanvasRect(const WindowInfo &window, const CanvasCamera &camera) {
    CanvasRect rect = rectToCanvasRect(window.normalRect);

    rect.x += camera.x;
    rect.y += camera.y;

    return rect;
}


