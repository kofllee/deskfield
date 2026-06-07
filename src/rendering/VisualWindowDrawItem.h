#pragma once

#include "workspace/WindowId.h"
#include "workspace/WorkspaceModel.h"

#include <d3d11.h>
#include <windows.h>

#include <string>

struct VisualWindowDrawItem {
    WindowId id{};

    RECT visualRect{};
    RECT titleRect{};
    RECT contentRect{};

    std::wstring title{};
    DeskfieldWindowState state{DeskfieldWindowState::Normal};

    ID3D11Texture2D* texture{};
    SIZE sourceSize{};

    bool selected{};
    bool focused{};

    bool captureAvailable{};
    bool captureFresh{};
    bool captureStale{};
    bool captureFailed{};
};