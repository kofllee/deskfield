#pragma once

#include "rendering/D3DDevice.h"

#include "workspace/CanvasTypes.h"
#include "workspace/WorkspaceModel.h"

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <windows.h>

class D3DCanvasRenderer {
public:
    D3DCanvasRenderer() = default;
    ~D3DCanvasRenderer();

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
    bool createSwapChain();
    bool createRenderTarget();
    void releaseRenderTarget();

    static int rectWidth(const RECT& rect);
    static int rectHeight(const RECT& rect);


private:
    HWND targetWindow_{nullptr};
    D3DDevice device_{};

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_{};
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_{};

    RECT clientRect_{};

    bool initialized_{false};
};
