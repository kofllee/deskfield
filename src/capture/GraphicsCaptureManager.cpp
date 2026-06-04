#include "GraphicsCaptureManager.h"

bool GraphicsCaptureManager::attach(WindowId id, HWND hwnd) {
    if (!id.isValid() || hwnd == nullptr) {
        return false;
    }

    CapturedWindow& captured = capturedWindows_[id];
    captured.id = id;
    captured.hwnd = hwnd;
    captured.attached = true;
    captured.frameAvailable = false;

    return true;
}

void GraphicsCaptureManager::detach(WindowId id) {
    capturedWindows_.erase(id);
}

void GraphicsCaptureManager::detachAll() {
    capturedWindows_.clear();
}

void GraphicsCaptureManager::update() {
    for (auto& [id, captured] : capturedWindows_) {
        captured.frameAvailable = captured.attached;
    }
}

bool GraphicsCaptureManager::isAttached(WindowId id) const {
    return capturedWindows_.find(id) != capturedWindows_.end();
}

const CapturedWindow* GraphicsCaptureManager::find(WindowId id) const {
    const auto it = capturedWindows_.find(id);

    if (it == capturedWindows_.end()) {
        return nullptr;
    }

    return &it->second;
}

