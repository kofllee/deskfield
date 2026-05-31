#pragma once

#include "WindowId.h"

#include <windows.h>

#include <unordered_map>

class WindowRegistry {
public:
    WindowId getOrCreate(HWND hwnd);

    WindowId findByHwnd(HWND hwnd) const;
    HWND findHwnd(WindowId id) const;

    void remove(HWND hwnd);
    void clear();

private:
    uint64_t nextId_{1};

    std::unordered_map<HWND, WindowId> hwndToId_{};
    std::unordered_map<WindowId, HWND> idToHwnd_{};
};