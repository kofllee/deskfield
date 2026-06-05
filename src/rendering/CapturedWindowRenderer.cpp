#include "CapturedWindowRenderer.h"

bool CapturedWindowRenderer::initialize(ID3D11Device* device) {
    if (device == nullptr) {
        return false;
    }

    initialized_ = true;
    return true;
}

void CapturedWindowRenderer::shutdown() {
    initialized_ = false;
}

void CapturedWindowRenderer::renderWindow(
    ID3D11DeviceContext* context,
    const CapturedWindow& capturedWindow,
    const CanvasRect&,
    const CanvasCamera&,
    const RECT&
) {
    if (!initialized_ || context == nullptr || !capturedWindow.attached) {
        return;
    }

}
