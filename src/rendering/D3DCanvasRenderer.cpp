#include "D3DCanvasRenderer.h"

bool D3DCanvasRenderer::initialize(HWND targetWindow) {
    if (targetWindow == nullptr || !IsWindow(targetWindow)) {
        return false;
    }

    targetWindow_ = targetWindow;

    if (!device_.initialize()) {
        targetWindow_ = nullptr;
        return false;
    }

    GetClientRect(targetWindow_, &clientRect_);

    initialized_ = true;
    return true;
}

void D3DCanvasRenderer::shutdown() {
    initialized_ = false;
    targetWindow_ = nullptr;
    clientRect_ = {};
}

void D3DCanvasRenderer::resize(const RECT& clientRect) {
    clientRect_ = clientRect;
}

void D3DCanvasRenderer::render(
    const WorkspaceModel&,
    const CanvasCamera&,
    const RECT&
) {
    if (!initialized_ || !device_.isValid()) {
        return;
    }
}

