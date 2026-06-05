#pragma once

#include "rendering/D3DDevice.h"

#include "workspace/CanvasTypes.h"
#include "workspace/WorkspaceModel.h"

#include <windows.h>

class D3DCanvasRenderer {
public:
    D3DCanvasRenderer() = default;
    ~D3DCanvasRenderer() = default;

    D3DCanvasRenderer(const D3DCanvasRenderer&) = delete;
    D3DCanvasRenderer& operator=(const D3DCanvasRenderer&) = delete;

    bool initialize(HWND targetWindow);
    void shutdown();

    void resize(const RECT& clientRect);

    void render(
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea
    );

    D3DDevice& device() {
        return device_;
    }

    const D3DDevice& device() const {
        return device_;
    }

    bool isInitialized() const {
        return initialized_;
    }

private:
    HWND targetWindow_{nullptr};
    D3DDevice device_{};

    RECT clientRect_{};
    bool initialized_{false};
};
