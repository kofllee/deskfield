#pragma once

#include <cstdint>
#include <functional>

struct WindowId {
    uint64_t value{};

    bool isValid() const {
        return value != 0;
    }

    friend bool operator==(const WindowId& a, const WindowId& b) {
        return a.value == b.value;
    }

    friend bool operator!=(const WindowId& a, const WindowId& b) {
        return a.value != b.value;
    }
};

namespace std {
    template <>
    struct hash<WindowId> {
        std::size_t operator()(const WindowId& id) const noexcept {
            return std::hash<uint64_t>()(id.value);
        }
    };
}