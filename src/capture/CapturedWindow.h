#pragma once

#include "workspace/WindowId.h"

#include <windows.h>

struct CapturedWindow {
    WindowId id{};
    HWND hwnd{};

    bool attached{};
    bool frameAvailable{};
};
