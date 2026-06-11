#include "WorkspaceModel.h"

#include <algorithm>

void WorkspaceModel::clear() {
    windows_.clear();
    selectedWindowId_ = {};
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

    window.state = snapshot.minimized
        ? DeskfieldWindowState::CanvasMinimized
        : DeskfieldWindowState::Normal;

    window.native = makeNativeState(snapshot);
    window.captureEnabled = true;

    windows_.push_back(window);
}

void WorkspaceModel::removeWindow(WindowId id) {
    CanvasWindow* window = findById(id);

    if (window != nullptr) {
        window->state = DeskfieldWindowState::Closed;
    }

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

    if (selectedWindowId_ == id) {
        selectedWindowId_ = {};
    }
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
    window->native = makeNativeState(snapshot);

    if (window->state == DeskfieldWindowState::Closed ||
        window->state == DeskfieldWindowState::Hidden ||
        window->state == DeskfieldWindowState::CanvasFullscreen ||
        window->state == DeskfieldWindowState::NativeInteractive) {
        return;
    }

    if (snapshot.minimized) {
        window->state = DeskfieldWindowState::CanvasMinimized;
        return;
    }

    if (window->state == DeskfieldWindowState::CanvasMinimized) {
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

    if (state == DeskfieldWindowState::CanvasFullscreen &&
        window->state != DeskfieldWindowState::CanvasFullscreen) {
        window->savedNormalCanvasRect = window->canvasRect;
    }

    window->state = state;
}

void WorkspaceModel::clearSelection() {
    for (CanvasWindow& window : windows_) {
        window.selected = false;
    }

    selectedWindowId_ = {};
}

void WorkspaceModel::selectWindow(WindowId id) {
    selectedWindowId_ = {};

    for (CanvasWindow& window : windows_) {
        const bool selected = window.id == id;
        window.selected = selected;

        if (selected) {
            selectedWindowId_ = id;
        }
    }
}

WindowId WorkspaceModel::selectedWindowId() const {
    return selectedWindowId_;
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
    RECT sourceRect = snapshot.rect;

    if (!snapshot.maximized && !snapshot.minimized) {
        sourceRect = snapshot.normalRect;
    }

    CanvasRect rect = rectToCanvasRect(sourceRect);

    rect.x += camera.x;
    rect.y += camera.y;

    return rect;
}

NativeSourceState WorkspaceModel::makeNativeState(
    const WindowSnapshot& snapshot
) {
    NativeSourceState state{};

    state.nativeRect = snapshot.rect;
    state.normalNativeRect = snapshot.normalRect;

    state.visible = snapshot.visible;
    state.minimized = snapshot.minimized;
    state.maximized = snapshot.maximized;
    state.cloaked = snapshot.cloaked;

    return state;
}