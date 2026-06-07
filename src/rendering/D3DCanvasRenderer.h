#pragma once

#include "D3DDevice.h"
#include "VisualWindowDrawItem.h"

#include "capture/GraphicsCaptureManager.h"
#include "workspace/CanvasTypes.h"
#include "workspace/ViewportMapper.h"
#include "workspace/WorkspaceModel.h"

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <wrl/client.h>

#include <vector>

struct TextureVertex {
    float x{};
    float y{};
    float u{};
    float v{};
};

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

    D3DDevice& device();
    const D3DDevice& device() const;

private:
    std::vector<VisualWindowDrawItem> buildVisualWindowDrawItems(
        const WorkspaceModel& workspace,
        const CanvasCamera& camera,
        const RECT& workArea,
        const ViewportMapper& mapper,
        const GraphicsCaptureManager& captureManager
    ) const;

    bool createSwapChain();
    bool createRenderTarget();
    void releaseRenderTarget();

    void drawCanvasGrid(
        const CanvasCamera& camera,
        const RECT& canvasArea
    );

    void drawVisualWindows(
        const std::vector<VisualWindowDrawItem>& items
    );

    void drawFilledRect(
        const RECT& rect,
        const float color[4]
    );

    void drawCapturedTexture(
        const VisualWindowDrawItem& item
    );

    bool ensureTexturePipeline();
    bool createTextureShaders();
    bool createTextureInputLayout();
    bool createTextureSamplers();
    void releaseTexturePipeline();

    static bool shouldUsePointSampling(
        const VisualWindowDrawItem& item
    );

    static RECT snapRectToPixels(
        const RECT& rect
    );

    static int rectWidth(const RECT& rect);
    static int rectHeight(const RECT& rect);

private:
    HWND targetWindow_{};

    D3DDevice device_{};

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_{};
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_{};

    Microsoft::WRL::ComPtr<ID3D11VertexShader> textureVertexShader_{};
    Microsoft::WRL::ComPtr<ID3D11PixelShader> texturePixelShader_{};
    Microsoft::WRL::ComPtr<ID3D11InputLayout> textureInputLayout_{};

    Microsoft::WRL::ComPtr<ID3D11SamplerState> textureLinearSampler_{};
    Microsoft::WRL::ComPtr<ID3D11SamplerState> texturePointSampler_{};

    RECT clientRect_{};

    bool initialized_{};
};