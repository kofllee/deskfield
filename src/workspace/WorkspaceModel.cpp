#include "WorkspaceModel.h"

void WorkspaceModel::rebuildFromWindows(const std::vector<WindowInfo>& windows) {
    windows_.clear();

    for (const WindowInfo& window : windows) {
        ManagedWindow managed{};
        managed.hwnd = window.hwnd;
        managed.canvasRect = rectToCanvasRect(window.rect);

        windows_.push_back(managed);
    }
}

std::vector<ManagedWindow>& WorkspaceModel::windows() { return windows_; }

const std::vector<ManagedWindow>& WorkspaceModel::windows() const { return windows_; }
