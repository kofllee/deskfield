#include "WindowStateTracker.h"

#include <algorithm>

void WindowStateTracker::update(std::vector<WindowSnapshot> snapshots) {
    previous_ = std::move(current_);
    current_ = std::move(snapshots);
}

const std::vector<WindowSnapshot>& WindowStateTracker::current() const {
    return current_;
}

const std::vector<WindowSnapshot>& WindowStateTracker::previous() const {
    return previous_;
}

std::vector<WindowSnapshot> WindowStateTracker::addedWindows() const {
    std::vector<WindowSnapshot> result{};

    for (const WindowSnapshot& snapshot : current_) {
        if (findByHwnd(previous_, snapshot.hwnd) == nullptr) {
            result.push_back(snapshot);
        }
    }

    return result;
}

std::vector<WindowSnapshot> WindowStateTracker::removedWindows() const {
    std::vector<WindowSnapshot> result{};

    for (const WindowSnapshot& snapshot : previous_) {
        if (findByHwnd(current_, snapshot.hwnd) == nullptr) {
            result.push_back(snapshot);
        }
    }

    return result;
}

std::vector<WindowSnapshot> WindowStateTracker::updatedWindows() const {
    std::vector<WindowSnapshot> result{};

    for (const WindowSnapshot& snapshot : current_) {
        const WindowSnapshot* previous = findByHwnd(previous_, snapshot.hwnd);

        if (previous == nullptr) {
            continue;
        }

        if (snapshot.title != previous->title ||
            snapshot.className != previous->className ||
            snapshot.processId != previous->processId ||
            snapshot.minimized != previous->minimized ||
            snapshot.maximized != previous->maximized ||
            snapshot.visible != previous->visible ||
            snapshot.cloaked != previous->cloaked) {
                result.push_back(snapshot);
            }
    }

    return result;
}

const WindowSnapshot* WindowStateTracker::findCurrent(HWND hwnd) const {
    return findByHwnd(current_, hwnd);
}

const WindowSnapshot* WindowStateTracker::findPrevious(HWND hwnd) const {
    return findByHwnd(previous_, hwnd);
}

const WindowSnapshot* WindowStateTracker::findByHwnd(
    const std::vector<WindowSnapshot>& snapshots,
    HWND hwnd
) {
    const auto it = std::find_if(
        snapshots.begin(),
        snapshots.end(),
        [hwnd](const WindowSnapshot& snapshot) {
            return snapshot.hwnd == hwnd;
        }
    );

    return it == snapshots.end() ? nullptr : &*it;
}