#include "WindowRegistry.h"

WindowId WindowRegistry::getOrCreate(HWND hwnd) {
    if (hwnd == nullptr) {
        return {};
    }

    const auto existing = hwndToId_.find(hwnd);

    if (existing != hwndToId_.end()) {
        return existing->second;
    }

    WindowId id{};
    id.value = nextId_++;

    hwndToId_.insert({hwnd, id});
    idToHwnd_.insert({id, hwnd});

    return id;
}

WindowId WindowRegistry::findByHwnd(HWND hwnd) const {
    const auto it = hwndToId_.find(hwnd);

    if (it == hwndToId_.end()) {
        return {};
    }

    return it->second;
}

HWND WindowRegistry::findHwnd(WindowId id) const {
    const auto it = idToHwnd_.find(id);

    if (it == idToHwnd_.end()) {
        return nullptr;
    }

    return it->second;
}

void WindowRegistry::remove(HWND hwnd) {
    const auto it = hwndToId_.find(hwnd);

    if (it == hwndToId_.end()) {
        return;
    }

    const WindowId id = it->second;

    hwndToId_.erase(it);
    idToHwnd_.erase(id);
}

void WindowRegistry::clear() {
    hwndToId_.clear();
    idToHwnd_.clear();
    nextId_ = 1;
}


