#include "WorkspaceModel.h"

#include <algorithm>

void WorkspaceModel::clear() {
    windows_.clear();
}

void WorkspaceModel::addWindow(
    const WindowSnapshot& snapshot,
    WindowId id,
    const CanvasCamera& camera
) {
    if (!id.isValid() || snapshot.hwnd == nullptr) {
        return;
    }

    if (findById(id) != nullptr) {
        updateMetadata(id, snapshot);
        updateNativeState(id, snapshot);
        return;
    }

    CanvasWindow window{};

    window.id = id;
    window.hwnd = snapshot.hwnd;

    window.title = snapshot.title;
    window.className = snapshot.className;
    window.processId = snapshot.processId;

    window.canvasRect = makeInitialCanvasRect(snapshot, camera);
    window.savedNormalCanvasRect = window.canvasRect;

    window.state = stateFromSnapshot(snapshot);
    window.captureEnabled = true;

    windows_.push_back(window);
}

void WorkspaceModel::removeWindow(WindowId id) {
    windows_.erase(
        std::remove_if(
            windows_.begin(),
            windows_.end(),
            [id](const CanvasWindow& window) {
                return window.id == id;
            }
        ),
        windows_.end()
    );
}

void WorkspaceModel::updateMetadata(
    WindowId id,
    const WindowSnapshot& snapshot
) {
    CanvasWindow* window = findById(id);

    if (window == nullptr) {
        return;
    }

    window->hwnd = snapshot.hwnd;
    window->title = snapshot.title;
    window->className = snapshot.className;
    window->processId = snapshot.processId;
}

void WorkspaceModel::updateNativeState(
    WindowId id,
    const WindowSnapshot& snapshot
) {
    CanvasWindow* window = findById(id);

    if (window == nullptr) {
        return;
    }

    updateMetadata(id, snapshot);

    if (snapshot.minimized) {
        window->state = DeskfieldWindowState::NativeMinimized;
        return;
    }

    if (snapshot.maximized) {
        if (window->state != DeskfieldWindowState::NativeMaximized) {
            window->savedNormalCanvasRect = window->canvasRect;
        }

        window->state = DeskfieldWindowState::NativeMaximized;
        return;
    }

    if (window->state == DeskfieldWindowState::NativeMaximized) {
        window->canvasRect = window->savedNormalCanvasRect;
    }

    if (window->state == DeskfieldWindowState::NativeMinimized ||
        window->state == DeskfieldWindowState::NativeMaximized) {
        window->state = DeskfieldWindowState::Normal;
        }
}

void WorkspaceModel::setCanvasRect(WindowId id, const CanvasRect& rect) {
    CanvasWindow* window = findById(id);

    if (window == nullptr) {
        return;
    }

    window->canvasRect = rect;
}

void WorkspaceModel::setState(WindowId id, DeskfieldWindowState state) {
    CanvasWindow* window = findById(id);

    if (window == nullptr) {
        return;
    }

    window->state = state;
}

CanvasWindow* WorkspaceModel::findById(WindowId id) {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [id](const CanvasWindow& window) {
            return window.id == id;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

const CanvasWindow* WorkspaceModel::findById(WindowId id) const {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [id](const CanvasWindow& window) {
            return window.id == id;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

CanvasWindow* WorkspaceModel::findByHwnd(HWND hwnd) {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hwnd](const CanvasWindow& window) {
            return window.hwnd == hwnd;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

const CanvasWindow* WorkspaceModel::findByHwnd(HWND hwnd) const {
    const auto it = std::find_if(
        windows_.begin(),
        windows_.end(),
        [hwnd](const CanvasWindow& window) {
            return window.hwnd == hwnd;
        }
    );

    return it == windows_.end() ? nullptr : &*it;
}

std::vector<CanvasWindow>& WorkspaceModel::windows() {
    return windows_;
}

const std::vector<CanvasWindow>& WorkspaceModel::windows() const {
    return windows_;
}

CanvasRect WorkspaceModel::makeInitialCanvasRect(
    const WindowSnapshot& snapshot,
    const CanvasCamera& camera
) {
    CanvasRect rect = rectToCanvasRect(snapshot.normalRect);

    rect.x += camera.x;
    rect.y += camera.y;

    return rect;
}

DeskfieldWindowState WorkspaceModel::stateFromSnapshot(
    const WindowSnapshot& snapshot
) {
    if (snapshot.minimized) {
        return DeskfieldWindowState::NativeMinimized;
    }

    if (snapshot.maximized) {
        return DeskfieldWindowState::NativeMaximized;
    }

    return DeskfieldWindowState::Normal;
}
