#pragma once

#include "workspace/WindowId.h"

#include <d3d11.h>
#include <windows.h>

#include <string>

struct VisualWindowDrawItem {
    WindowId id{};
    RECT visualRect{};

    std::wstring title{};

    ID3D11Texture2D* texture{};
    SIZE sourceSize{};

    bool selected{};
    bool focused{};
    bool captureAvailable{};
};