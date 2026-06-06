#include "D3DCanvasRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
    constexpr float BackgroundColor[] = {
        0.031f,
        0.039f,
        0.055f,
        1.0f
    };
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

    releaseRenderTarget();

    if (swapChain_ != nullptr) {
        swapChain_->SetFullscreenState(FALSE, nullptr);
        swapChain_.Reset();
    }

    targetWindow_ = nullptr;
    clientRect_ = {};
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
    const ViewportMapper& mapper
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

    drawCanvasGrid();

    const auto items = buildVisualWindowDrawItems(
        workspace,
        camera,
        workArea,
        mapper
    );

    drawVisualWindows(items);

    swapChain_->Present(1, 0);
}

std::vector<VisualWindowDrawItem> D3DCanvasRenderer::buildVisualWindowDrawItems(
    const WorkspaceModel& workspace,
    const CanvasCamera& camera,
    const RECT& workArea,
    const ViewportMapper& mapper
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

        if (visualRect.right <= 0 ||
            visualRect.bottom <= 0 ||
            visualRect.left >= rectWidth(clientRect_) ||
            visualRect.top >= rectHeight(clientRect_)) {
            continue;
            }

        VisualWindowDrawItem item{};
        item.id = window.id;
        item.visualRect = visualRect;
        item.title = window.title;
        item.selected = window.selected;
        item.focused = false;
        item.captureAvailable = window.captureEnabled;

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

void D3DCanvasRenderer::drawCanvasGrid() {
    constexpr float minorGridColor[] = {0.075f, 0.085f, 0.11f, 1.0f};
    constexpr int gridStep = 64;

    const int width = rectWidth(clientRect_);
    const int height = rectHeight(clientRect_);

    for (int x = 0; x < width; x += gridStep) {
        RECT line{};
        line.left = x;
        line.top = 0;
        line.right = x + 1;
        line.bottom = height;

        drawFilledRect(line, minorGridColor);
    }

    for (int y = 0; y < height; y += gridStep) {
        RECT line{};
        line.left = 0;
        line.top = y;
        line.right = width;
        line.bottom = y + 1;

        drawFilledRect(line, minorGridColor);
    }
}

void D3DCanvasRenderer::drawVisualWindows(
    const std::vector<VisualWindowDrawItem>& items
) {
    constexpr float windowColor[] = {0.105f, 0.125f, 0.17f, 1.0f};
    constexpr float titleColor[] = {0.145f, 0.175f, 0.235f, 1.0f};
    constexpr float borderColor[] = {0.33f, 0.48f, 0.82f, 1.0f};

    for (const VisualWindowDrawItem& item : items) {
        RECT rect = item.visualRect;

        drawFilledRect(rect, windowColor);

        RECT titleRect = rect;
        titleRect.bottom = std::min(titleRect.top + 28, titleRect.bottom);
        drawFilledRect(titleRect, titleColor);

        RECT top{rect.left, rect.top, rect.right, rect.top + 2};
        RECT bottom{rect.left, rect.bottom - 2, rect.right, rect.bottom};
        RECT left{rect.left, rect.top, rect.left + 2, rect.bottom};
        RECT right{rect.right - 2, rect.top, rect.right, rect.bottom};

        drawFilledRect(top, borderColor);
        drawFilledRect(bottom, borderColor);
        drawFilledRect(left, borderColor);
        drawFilledRect(right, borderColor);
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

