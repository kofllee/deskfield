#include "D3DCanvasRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <d3dcompiler.h>

namespace {
    constexpr float BackgroundColor[] = {
        0.031f,
        0.039f,
        0.055f,
        1.0f
    };

    constexpr int TitleBarHeight = 30;
    constexpr int BorderThickness = 2;

    constexpr char TextureVertexShaderSource[] = R"(
    struct VSInput {
        float2 position : POSITION;
        float2 uv : TEXCOORD0;
    };

    struct VSOutput {
        float4 position : SV_POSITION;
        float2 uv : TEXCOORD0;
    };

    VSOutput main(VSInput input) {
        VSOutput output;
        output.position = float4(input.position, 0.0f, 1.0f);
        output.uv = input.uv;
        return output;
    }
    )";

    constexpr char TexturePixelShaderSource[] = R"(
    Texture2D sourceTexture : register(t0);
    SamplerState sourceSampler : register(s0);

    struct PSInput {
        float4 position : SV_POSITION;
        float2 uv : TEXCOORD0;
    };

    float4 main(PSInput input) : SV_TARGET {
        return sourceTexture.Sample(sourceSampler, input.uv);
    }
    )";

    bool isEmptyRect(const RECT& rect) {
        return rect.left >= rect.right || rect.top >= rect.bottom;
    }
}

D3DCanvasRenderer::~D3DCanvasRenderer() {
    shutdown();
}

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

    if (!createSwapChain()) {
        shutdown();
        return false;
    }

    if (!createRenderTarget()) {
        shutdown();
        return false;
    }

    initialized_ = true;
    return true;
}

void D3DCanvasRenderer::shutdown() {
    initialized_ = false;

    releaseTexturePipeline();
    releaseRenderTarget();

    if (swapChain_ != nullptr) {
        swapChain_->SetFullscreenState(FALSE, nullptr);
        swapChain_.Reset();
    }

    targetWindow_ = nullptr;
    clientRect_ = {};
}

D3DDevice& D3DCanvasRenderer::device() {
    return device_;
}

const D3DDevice& D3DCanvasRenderer::device() const {
    return device_;
}

void D3DCanvasRenderer::resize(const RECT& clientRect) {
    clientRect_ = clientRect;

    if (!initialized_ || swapChain_ == nullptr) {
        return;
    }

    const int width = rectWidth(clientRect_);
    const int height = rectHeight(clientRect_);

    if (width <= 0 || height <= 0) {
        return;
    }

    releaseRenderTarget();

    const HRESULT resizeResult = swapChain_->ResizeBuffers(
        0,
        static_cast<UINT>(width),
        static_cast<UINT>(height),
        DXGI_FORMAT_UNKNOWN,
        0
    );

    if (FAILED(resizeResult)) {
        initialized_ = false;
        return;
    }

    if (!createRenderTarget()) {
        initialized_ = false;
    }
}

void D3DCanvasRenderer::render(
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const GraphicsCaptureManager& captureManager
) {
    if (!initialized_ || !device_.isValid() || renderTargetView_ == nullptr) {
        return;
    }

    ID3D11DeviceContext* context = device_.context();

    context->OMSetRenderTargets(
        1,
        renderTargetView_.GetAddressOf(),
        nullptr
    );

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(rectWidth(clientRect_));
    viewport.Height = static_cast<float>(rectHeight(clientRect_));
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    context->RSSetViewports(1, &viewport);

    context->ClearRenderTargetView(
        renderTargetView_.Get(),
        BackgroundColor
    );

    RECT canvasArea{};
    canvasArea.left = 0;
    canvasArea.top = 0;
    canvasArea.right = rectWidth(clientRect_);
    canvasArea.bottom = rectHeight(clientRect_);

    drawCanvasGrid(camera, canvasArea);

    const auto items = buildVisualWindowDrawItems(
        workspace,
        camera,
        canvasArea,
        mapper,
        captureManager
    );

    drawVisualWindows(items);

    swapChain_->Present(1, 0);
}

std::vector<VisualWindowDrawItem> D3DCanvasRenderer::buildVisualWindowDrawItems(
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper,
    const GraphicsCaptureManager& captureManager
) const {
    std::vector<VisualWindowDrawItem> items;

    for (const CanvasWindow& window : workspace.windows()) {
        if (window.state == DeskfieldWindowState::Closed ||
            window.state == DeskfieldWindowState::Hidden) {
            continue;
        }

        RECT visualRect = mapper.mapCanvasToVisualRect(
            window.canvasRect,
            camera,
            workArea
        );

        visualRect.top -= static_cast<LONG>(
            static_cast<double>(TitleBarHeight) * camera.zoom
        );

        if (visualRect.right <= 0 ||
            visualRect.bottom <= 0 ||
            visualRect.left >= rectWidth(clientRect_) ||
            visualRect.top >= rectHeight(clientRect_)) {
            continue;
        }

        const int scaledTitleHeight = std::max(
            18,
            static_cast<int>(static_cast<double>(TitleBarHeight) * camera.zoom)
        );

        RECT titleRect = visualRect;
        titleRect.bottom = std::min<LONG>(
            titleRect.top + scaledTitleHeight,
            visualRect.bottom
        );

        RECT contentRect = visualRect;
        contentRect.top = titleRect.bottom;

        if (isEmptyRect(contentRect)) {
            continue;
        }

        const CaptureStatus captureStatus = captureManager.captureStatus(window.id);

        VisualWindowDrawItem item{};
        item.id = window.id;

        item.visualRect = visualRect;
        item.titleRect = titleRect;
        item.contentRect = contentRect;

        item.title = window.title;
        item.state = window.state;

        item.texture = captureManager.latestTexture(window.id);
        item.sourceSize = captureManager.sourceSize(window.id);

        item.selected = window.selected;
        item.focused = window.state == DeskfieldWindowState::NativeInteractive;

        item.captureAvailable = captureStatus.hasTexture;
        item.captureFresh = captureStatus.fresh;
        item.captureStale = captureStatus.stale;
        item.captureFailed = captureStatus.failed;

        items.push_back(std::move(item));
    }

    return items;
}

bool D3DCanvasRenderer::createSwapChain() {
    if (!device_.isValid() || targetWindow_ == nullptr) {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;

    HRESULT result = device_.device()->QueryInterface(
        __uuidof(IDXGIDevice),
        reinterpret_cast<void**>(dxgiDevice.GetAddressOf())
    );

    if (FAILED(result)) {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

    result = dxgiDevice->GetAdapter(adapter.GetAddressOf());

    if (FAILED(result)) {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> factory;

    result = adapter->GetParent(
        __uuidof(IDXGIFactory),
        reinterpret_cast<void**>(factory.GetAddressOf())
    );

    if (FAILED(result)) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferDesc.Width = static_cast<UINT>(rectWidth(clientRect_));
    desc.BufferDesc.Height = static_cast<UINT>(rectHeight(clientRect_));
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = targetWindow_;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    result = factory->CreateSwapChain(
        device_.device(),
        &desc,
        swapChain_.GetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    factory->MakeWindowAssociation(
        targetWindow_,
        DXGI_MWA_NO_ALT_ENTER
    );

    return true;
}

bool D3DCanvasRenderer::createRenderTarget() {
    if (swapChain_ == nullptr || !device_.isValid()) {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    HRESULT result = swapChain_->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf())
    );

    if (FAILED(result)) {
        return false;
    }

    result = device_.device()->CreateRenderTargetView(
        backBuffer.Get(),
        nullptr,
        renderTargetView_.GetAddressOf()
    );

    return SUCCEEDED(result);
}

void D3DCanvasRenderer::releaseRenderTarget() {
    if (device_.context() != nullptr) {
        ID3D11RenderTargetView* emptyTargets[] = {nullptr};
        device_.context()->OMSetRenderTargets(1, emptyTargets, nullptr);
    }

    renderTargetView_.Reset();
}

int D3DCanvasRenderer::rectWidth(const RECT& rect) {
    return rect.right - rect.left;
}

int D3DCanvasRenderer::rectHeight(const RECT& rect) {
    return rect.bottom - rect.top;
}

void D3DCanvasRenderer::drawCanvasGrid(
    const CanvasCamera& camera,
    const RECT& canvasArea
) {
    constexpr float minorGridColor[] = {0.075f, 0.085f, 0.11f, 1.0f};
    constexpr int gridStep = 128;

    const int width = rectWidth(clientRect_);
    const int height = rectHeight(clientRect_);

    const double scaledStep = static_cast<double>(gridStep) * camera.zoom;

    if (scaledStep < 8.0) {
        return;
    }

    const double originX =
        static_cast<double>(canvasArea.left) -
        camera.x * camera.zoom;

    const double originY =
        static_cast<double>(canvasArea.top) -
        camera.y * camera.zoom;

    double startX = std::fmod(originX, scaledStep);
    double startY = std::fmod(originY, scaledStep);

    if (startX < 0.0) {
        startX += scaledStep;
    }

    if (startY < 0.0) {
        startY += scaledStep;
    }

    for (double x = startX; x < width; x += scaledStep) {
        RECT line{};
        line.left = static_cast<LONG>(x);
        line.top = 0;
        line.right = line.left + 1;
        line.bottom = height;

        drawFilledRect(line, minorGridColor);
    }

    for (double y = startY; y < height; y += scaledStep) {
        RECT line{};
        line.left = 0;
        line.top = static_cast<LONG>(y);
        line.right = width;
        line.bottom = line.top + 1;

        drawFilledRect(line, minorGridColor);
    }
}

void D3DCanvasRenderer::drawVisualWindows(
    const std::vector<VisualWindowDrawItem>& items
) {
    constexpr float windowColor[] = {0.105f, 0.125f, 0.17f, 1.0f};
    constexpr float minimizedColor[] = {0.075f, 0.085f, 0.115f, 1.0f};

    constexpr float titleColor[] = {0.145f, 0.175f, 0.235f, 1.0f};
    constexpr float staleTitleColor[] = {0.165f, 0.145f, 0.105f, 1.0f};

    constexpr float borderColor[] = {0.33f, 0.48f, 0.82f, 1.0f};
    constexpr float selectedBorderColor[] = {0.52f, 0.68f, 1.0f, 1.0f};

    for (const VisualWindowDrawItem& item : items) {
        const bool minimized =
            item.state == DeskfieldWindowState::CanvasMinimized;

        drawFilledRect(
            item.visualRect,
            minimized ? minimizedColor : windowColor
        );

        if (!minimized && item.captureAvailable && item.texture != nullptr) {
            drawCapturedTexture(item);
        } else {
            drawFilledRect(
                item.contentRect,
                minimized ? minimizedColor : windowColor
            );
        }

        drawFilledRect(
            item.titleRect,
            item.captureStale || item.captureFailed
                ? staleTitleColor
                : titleColor
        );

        const float* currentBorderColor =
            item.selected || item.focused
                ? selectedBorderColor
                : borderColor;

        const RECT rect = item.visualRect;

        RECT top{
            rect.left,
            rect.top,
            rect.right,
            rect.top + BorderThickness
        };

        RECT bottom{
            rect.left,
            rect.bottom - BorderThickness,
            rect.right,
            rect.bottom
        };

        RECT left{
            rect.left,
            rect.top,
            rect.left + BorderThickness,
            rect.bottom
        };

        RECT right{
            rect.right - BorderThickness,
            rect.top,
            rect.right,
            rect.bottom
        };

        drawFilledRect(top, currentBorderColor);
        drawFilledRect(bottom, currentBorderColor);
        drawFilledRect(left, currentBorderColor);
        drawFilledRect(right, currentBorderColor);
    }
}

void D3DCanvasRenderer::drawFilledRect(const RECT& rect, const float color[4]) {
    if (renderTargetView_ == nullptr || !device_.isValid()) {
        return;
    }

    RECT clipped{};
    clipped.left = std::max<LONG>(0, rect.left);
    clipped.top = std::max<LONG>(0, rect.top);
    clipped.right = std::min<LONG>(rectWidth(clientRect_), rect.right);
    clipped.bottom = std::min<LONG>(rectHeight(clientRect_), rect.bottom);

    if (clipped.left >= clipped.right || clipped.top >= clipped.bottom) {
        return;
    }

    D3D11_RECT d3dRect{};
    d3dRect.left = clipped.left;
    d3dRect.top = clipped.top;
    d3dRect.right = clipped.right;
    d3dRect.bottom = clipped.bottom;

    ID3D11DeviceContext1* context1 = device_.context1();

    if (context1 == nullptr) {
        return;
    }

    context1->ClearView(
        renderTargetView_.Get(),
        color,
        &d3dRect,
        1
    );
}

void D3DCanvasRenderer::drawCapturedTexture(const VisualWindowDrawItem& item) {
    if (item.texture == nullptr || !ensureTexturePipeline()) {
        return;
    }

    const RECT originalRect = snapRectToPixels(item.contentRect);

    RECT clippedRect = originalRect;

    clippedRect.left = std::max<LONG>(0, clippedRect.left);
    clippedRect.top = std::max<LONG>(0, clippedRect.top);
    clippedRect.right = std::min<LONG>(rectWidth(clientRect_), clippedRect.right);
    clippedRect.bottom = std::min<LONG>(rectHeight(clientRect_), clippedRect.bottom);

    if (clippedRect.left >= clippedRect.right ||
        clippedRect.top >= clippedRect.bottom) {
        return;
    }

    const float originalWidth =
        static_cast<float>(originalRect.right - originalRect.left);

    const float originalHeight =
        static_cast<float>(originalRect.bottom - originalRect.top);

    if (originalWidth <= 0.0f || originalHeight <= 0.0f) {
        return;
    }

    const float u0 =
        static_cast<float>(clippedRect.left - originalRect.left) / originalWidth;

    const float v0 =
        static_cast<float>(clippedRect.top - originalRect.top) / originalHeight;

    const float u1 =
        static_cast<float>(clippedRect.right - originalRect.left) / originalWidth;

    const float v1 =
        static_cast<float>(clippedRect.bottom - originalRect.top) / originalHeight;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

    const HRESULT srvResult = device_.device()->CreateShaderResourceView(
        item.texture,
        nullptr,
        textureView.GetAddressOf()
    );

    if (FAILED(srvResult)) {
        return;
    }

    const float width = static_cast<float>(rectWidth(clientRect_));
    const float height = static_cast<float>(rectHeight(clientRect_));

    if (width <= 0.0f || height <= 0.0f) {
        return;
    }

    auto toNdcX = [width](LONG x) {
        return (static_cast<float>(x) / width) * 2.0f - 1.0f;
    };

    auto toNdcY = [height](LONG y) {
        return 1.0f - (static_cast<float>(y) / height) * 2.0f;
    };

    TextureVertex vertices[] = {
        {toNdcX(clippedRect.left),  toNdcY(clippedRect.top),    u0, v0},
        {toNdcX(clippedRect.right), toNdcY(clippedRect.top),    u1, v0},
        {toNdcX(clippedRect.left),  toNdcY(clippedRect.bottom), u0, v1},
        {toNdcX(clippedRect.right), toNdcY(clippedRect.bottom), u1, v1},
    };

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA bufferData{};
    bufferData.pSysMem = vertices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    const HRESULT bufferResult = device_.device()->CreateBuffer(
        &bufferDesc,
        &bufferData,
        vertexBuffer.GetAddressOf()
    );

    if (FAILED(bufferResult)) {
        return;
    }

    constexpr UINT stride = sizeof(TextureVertex);
    constexpr UINT offset = 0;

    ID3D11DeviceContext* context = device_.context();

    context->IASetInputLayout(textureInputLayout_.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ID3D11Buffer* vertexBuffers[] = {
        vertexBuffer.Get()
    };

    context->IASetVertexBuffers(
        0,
        1,
        vertexBuffers,
        &stride,
        &offset
    );

    context->VSSetShader(textureVertexShader_.Get(), nullptr, 0);
    context->PSSetShader(texturePixelShader_.Get(), nullptr, 0);

    ID3D11ShaderResourceView* srv = textureView.Get();
    context->PSSetShaderResources(0, 1, &srv);

    ID3D11SamplerState* sampler =
        shouldUsePointSampling(item)
            ? texturePointSampler_.Get()
            : textureLinearSampler_.Get();

    context->PSSetSamplers(0, 1, &sampler);

    context->Draw(4, 0);

    ID3D11ShaderResourceView* emptySrv[] = {nullptr};
    context->PSSetShaderResources(0, 1, emptySrv);
}

bool D3DCanvasRenderer::createTextureShaders() {
    Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT result = D3DCompile(
        TextureVertexShaderSource,
        std::strlen(TextureVertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        vertexBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    result = D3DCompile(
        TexturePixelShaderSource,
        std::strlen(TexturePixelShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        pixelBlob.GetAddressOf(),
        errorBlob.ReleaseAndGetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    result = device_.device()->CreateVertexShader(
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        nullptr,
        textureVertexShader_.GetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    result = device_.device()->CreatePixelShader(
        pixelBlob->GetBufferPointer(),
        pixelBlob->GetBufferSize(),
        nullptr,
        texturePixelShader_.GetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            sizeof(float) * 2,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    result = device_.device()->CreateInputLayout(
        layout,
        2,
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        textureInputLayout_.GetAddressOf()
    );

    return SUCCEEDED(result);
}

bool D3DCanvasRenderer::createTextureInputLayout() {
    return textureInputLayout_ != nullptr;
}

bool D3DCanvasRenderer::createTextureSamplers() {
    D3D11_SAMPLER_DESC linearDesc{};
    linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    linearDesc.MinLOD = 0;
    linearDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT result = device_.device()->CreateSamplerState(
        &linearDesc,
        textureLinearSampler_.GetAddressOf()
    );

    if (FAILED(result)) {
        return false;
    }

    D3D11_SAMPLER_DESC pointDesc{};
    pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    pointDesc.MinLOD = 0;
    pointDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device_.device()->CreateSamplerState(
        &pointDesc,
        texturePointSampler_.GetAddressOf()
    );

    return SUCCEEDED(result);
}

void D3DCanvasRenderer::releaseTexturePipeline() {
    texturePointSampler_.Reset();
    textureLinearSampler_.Reset();

    textureInputLayout_.Reset();
    texturePixelShader_.Reset();
    textureVertexShader_.Reset();
}

bool D3DCanvasRenderer::ensureTexturePipeline() {
    if (textureVertexShader_ != nullptr &&
        texturePixelShader_ != nullptr &&
        textureInputLayout_ != nullptr &&
        textureLinearSampler_ != nullptr &&
        texturePointSampler_ != nullptr) {
        return true;
        }

    releaseTexturePipeline();

    if (!createTextureShaders()) {
        releaseTexturePipeline();
        return false;
    }

    if (!createTextureInputLayout()) {
        releaseTexturePipeline();
        return false;
    }

    if (!createTextureSamplers()) {
        releaseTexturePipeline();
        return false;
    }

    return true;
}

bool D3DCanvasRenderer::shouldUsePointSampling(
    const VisualWindowDrawItem& item
) {
    if (item.sourceSize.cx <= 0 || item.sourceSize.cy <= 0) {
        return true;
    }

    const int drawWidth = rectWidth(item.contentRect);
    const int drawHeight = rectHeight(item.contentRect);

    if (drawWidth <= 0 || drawHeight <= 0) {
        return true;
    }

    const double scaleX =
        static_cast<double>(drawWidth) /
        static_cast<double>(item.sourceSize.cx);

    const double scaleY =
        static_cast<double>(drawHeight) /
        static_cast<double>(item.sourceSize.cy);

    const double scale = std::min(scaleX, scaleY);

    return scale >= 0.90;
}

RECT D3DCanvasRenderer::snapRectToPixels(
    const RECT& rect
) {
    RECT snapped{};

    snapped.left = rect.left;
    snapped.top = rect.top;
    snapped.right = rect.right;
    snapped.bottom = rect.bottom;

    return snapped;
}