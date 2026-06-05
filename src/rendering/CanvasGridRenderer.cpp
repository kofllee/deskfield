#include "CanvasGridRenderer.h"

bool CanvasGridRenderer::initialize(ID3D11Device* device) {
    if (device == nullptr) {
        return false;
    }

    initialized_ = true;
    return true;
}

void CanvasGridRenderer::shutdown() {
    initialized_ = false;
}

void CanvasGridRenderer::render(
    ID3D11DeviceContext* context,
    const CanvasCamera&,
    const RECT&
) {
    if (!initialized_ || context == nullptr) {
        return;
    }
}