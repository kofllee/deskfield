#pragma once

#include "rendering/D3DDevice.h"
#include "rendering/VisualWindowDrawItem.h"

#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"
#include "capture/GraphicsCaptureManager.h"

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <windows.h>

#include <vector>

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
        const RECT& workArea,
        const ViewportMapper& mapper,
        const GraphicsCaptureManager& captureManager
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
    struct TextureVertex {
        float x;
        float y;
        float u;
        float v;
    };

    bool createSwapChain();
    bool createRenderTarget();
    void releaseRenderTarget();

    std::vector<VisualWindowDrawItem> buildVisualWindowDrawItems(
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& canvasArea,
        const ViewportMapper& mapper,
        const GraphicsCaptureManager& captureManager
    ) const;

    void drawCanvasGrid(
        const CanvasCamera& camera,
        const RECT& canvasArea
    );
    void drawVisualWindows(const std::vector<VisualWindowDrawItem>& items);
    void drawFilledRect(const RECT& rect, const float color[4]);

    static int rectWidth(const RECT& rect);
    static int rectHeight(const RECT& rect);

    void drawCapturedTexture(const VisualWindowDrawItem& item);
    bool ensureTexturePipeline();
    bool createTextureShaders();
    bool createTextureSampler();
    bool createTextureInputLayout();
    void releaseTexturePipeline();

private:
    HWND targetWindow_{nullptr};
    D3DDevice device_{};

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_{};
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_{};

    Microsoft::WRL::ComPtr<ID3D11VertexShader> textureVertexShader_{};
    Microsoft::WRL::ComPtr<ID3D11PixelShader> texturePixelShader_{};
    Microsoft::WRL::ComPtr<ID3D11InputLayout> textureInputLayout_{};
    Microsoft::WRL::ComPtr<ID3D11SamplerState> textureSampler_{};

    RECT clientRect_{};

    bool initialized_{false};
};