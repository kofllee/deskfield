#pragma once

#include "WindowSnapshot.h"

#include <windows.h>

#include <vector>

class WindowStateTracker {
public:
    void update(std::vector<WindowSnapshot> snapshots);

    const std::vector<WindowSnapshot>& current() const;
    const std::vector<WindowSnapshot>& previous() const;

    std::vector<WindowSnapshot> addedWindows() const;
    std::vector<WindowSnapshot> removedWindows() const;
    std::vector<WindowSnapshot> updatedWindows() const;

    const WindowSnapshot* findCurrent(HWND hwnd) const;
    const WindowSnapshot* findPrevious(HWND hwnd) const;

private:
    static const WindowSnapshot* findByHwnd(
        const std::vector<WindowSnapshot>& snapshots,
        HWND hwnd
    );

private:
    std::vector<WindowSnapshot> previous_{};
    std::vector<WindowSnapshot> current_{};
};