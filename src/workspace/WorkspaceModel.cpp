#include "WorkspaceModel.h"

#include <algorithm>

void WorkspaceModel::rebuildFromWindows(
    const std::vector<WindowSnapshot>& windows,
    WindowRegistry& registry
) {
    windows_.clear();

    CanvasCamera camera{};
    camera.x = 0.0;
    camera.y = 0.0;
    camera.zoom = 1.0;

    for (const WindowSnapshot& window : windows) {
        const WindowId id = registry.getOrCreate(window.hwnd);
        windows_.push_back(makeManagedWindow(window, camera, id));
    }
}

void WorkspaceModel::syncFromWindows(
    const std::vector<WindowSnapshot>& windows,
    const CanvasCamera& camera,
    WindowRegistry& registry
) {
    windows_.erase(
        std::remove_if(
            windows_.begin(),
            windows_.end(),
            [&](const ManagedWindow& managed) {
                const bool shouldRemove =
                    managed.hwnd == nullptr ||
                    !IsWindow(managed.hwnd) ||
                    !containsWindow(windows, managed.hwnd);

                if (shouldRemove) {
                    registry.remove(managed.hwnd);
                }

                return shouldRemove;
            }
        ),
        windows_.end()
    );

    for (const WindowSnapshot& window : windows) {
        ManagedWindow* existing = findByHwnd(window.hwnd);

        if (existing != nullptr) {
            existing->title = window.title;
            existing->className = window.className;
            existing->processId = window.processId;
            continue;
        }

        const WindowId id = registry.getOrCreate(window.hwnd);
        windows_.push_back(makeManagedWindow(window, camera, id));
    }
}

void WorkspaceModel::updateNativeState(const std::vector<WindowSnapshot>& windows) {
    for (ManagedWindow& managed : windows_) {
        const auto it = std::find_if(
            windows.begin(),
            windows.end(),
            [&](const WindowSnapshot& window) {
                return window.hwnd == managed.hwnd;
            }
        );

        if (it == windows.end()) {
            continue;
        }

        const WindowSnapshot& window = *it;

        managed.title = window.title;
        managed.className = window.className;
        managed.processId = window.processId;

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
            managed.wasMinimized = false;
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

std::vector<ManagedWindow>& WorkspaceModel::windows() {
    return windows_;
}

const std::vector<ManagedWindow>& WorkspaceModel::windows() const {
    return windows_;
}

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

bool WorkspaceModel::containsWindow(
    const std::vector<WindowSnapshot>& windows,
    HWND hwnd
) {
    return std::any_of(
        windows.begin(),
        windows.end(),
        [hwnd](const WindowSnapshot& window) {
            return window.hwnd == hwnd;
        }
    );
}

CanvasRect WorkspaceModel::makeInitialCanvasRect(
    const WindowSnapshot& window,
    const CanvasCamera& camera
) {
    CanvasRect rect = rectToCanvasRect(window.normalRect);

    rect.x += camera.x;
    rect.y += camera.y;

    return rect;
}

ManagedWindow WorkspaceModel::makeManagedWindow(
    const WindowSnapshot& window,
    const CanvasCamera& camera,
    WindowId id
) {
    ManagedWindow managed{};

    managed.id = id;
    managed.hwnd = window.hwnd;

    managed.title = window.title;
    managed.className = window.className;
    managed.processId = window.processId;

    managed.canvasRect = makeInitialCanvasRect(window, camera);
    managed.savedNormalRect = managed.canvasRect;

    managed.wasMinimized = window.minimized;
    managed.wasMaximized = window.maximized;

    if (window.minimized) {
        managed.state = ManagedWindowState::Minimized;
    } else if (window.maximized) {
        managed.state = ManagedWindowState::Maximized;
    } else {
        managed.state = ManagedWindowState::Normal;
    }

    return managed;
}