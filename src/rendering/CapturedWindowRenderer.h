#pragma once

#include "capture/CapturedWindow.h"

#include "workspace/CanvasTypes.h"

#include <d3d11.h>
#include <windows.h>

class CapturedWindowRenderer {
public:
    CapturedWindowRenderer() = default;
    ~CapturedWindowRenderer() = default;

    bool initialize(ID3D11Device* device);
    void shutdown();

    void renderWindow(
        ID3D11DeviceContext* context,
        const CapturedWindow& capturedWindow,
        const CanvasRect& canvasRect,
        const CanvasCamera& camera,
        const RECT& workArea
    );

    bool isInitialized() const {
        return initialized_;
    }

private:
    bool initialized_{false};
};
