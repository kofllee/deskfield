#pragma once

#include "workspace/CanvasTypes.h"

#include <d3d11.h>
#include <windows.h>

class CanvasGridRenderer {
public:
    CanvasGridRenderer() = default;
    ~CanvasGridRenderer() = default;

    bool initialize(ID3D11Device* device);
    void shutdown();

    void render(
        ID3D11DeviceContext* context,
        const CanvasCamera& camera,
        const RECT& viewport
    );

    bool isInitialized() const {
        return initialized_;
    }

private:
    bool initialized_{false};

};