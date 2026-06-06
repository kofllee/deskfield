#pragma once

#include "workspace/WindowId.h"

#include <windows.h>

#include <string>

struct VisualWindowDrawItem {
    WindowId id{};
    RECT visualRect{};

    std::wstring title{};

    bool selected{};
    bool focused{};
    bool captureAvailable{};
};
