#include "GraphicsCaptureManager.h"

#include "platform/Direct3DWinRtInterop.h"
#include "platform/GraphicsCaptureInterop.h"

#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <wrl/client.h>

#include <utility>

bool GraphicsCaptureManager::initialize(ID3D11Device* d3dDevice) {
    winrtDevice = createDirect3DDeviceFromD3D11Device(d3dDevice);
    return winrtDevice != nullptr;
}

void GraphicsCaptureManager::shutdown() {
    detachAll();
    winrtDevice = nullptr;
}

bool GraphicsCaptureManager::attach(WindowId id, HWND hwnd) {
    if (!id.isValid() || hwnd == nullptr || !IsWindow(hwnd) || winrtDevice == nullptr) {
        return false;
    }

    if (isAttached(id)) {
        return true;
    }

    CapturedWindow captured{};
    captured.id = id;
    captured.hwnd = hwnd;
    captured.attached = true;

    auto [it, inserted] = capturedWindows_.emplace(id, std::move(captured));

    if (!inserted) {
        return false;
    }

    if (!createCaptureForWindow(it->second)) {
        capturedWindows_.erase(it);
        return false;
    }

    return true;
}

void GraphicsCaptureManager::detach(WindowId id) {
    dirtyWindows_.erase(id);

    const auto it = capturedWindows_.find(id);

    if (it == capturedWindows_.end()) {
        return;
    }

    closeCapture(it->second);
    capturedWindows_.erase(it);
}

void GraphicsCaptureManager::detachAll() {
    dirtyWindows_.clear();

    for (auto& [id, captured] : capturedWindows_) {
        closeCapture(captured);
    }

    capturedWindows_.clear();
}

void GraphicsCaptureManager::update() {
    if (dirtyWindows_.empty()) {
        return;
    }

    auto dirty = std::move(dirtyWindows_);
    dirtyWindows_.clear();

    for (WindowId id : dirty) {
        auto it = capturedWindows_.find(id);

        if (it == capturedWindows_.end()) {
            continue;
        }

        drainFrames(it->second);
    }
}

bool GraphicsCaptureManager::isAttached(WindowId id) const {
    return capturedWindows_.find(id) != capturedWindows_.end();
}

const CapturedWindow* GraphicsCaptureManager::find(WindowId id) const {
    const auto it = capturedWindows_.find(id);

    if (it == capturedWindows_.end()) {
        return nullptr;
    }

    return &it->second;
}

ID3D11Texture2D* GraphicsCaptureManager::latestTexture(WindowId id) const {
    const CapturedWindow* captured = find(id);

    if (captured == nullptr || !captured->frameAvailable) {
        return nullptr;
    }

    return captured->latestTexture.Get();
}

SIZE GraphicsCaptureManager::sourceSize(WindowId id) const {
    const CapturedWindow* captured = find(id);

    if (captured == nullptr) {
        return {};
    }

    return captured->sourceSize;
}

bool GraphicsCaptureManager::createCaptureForWindow(CapturedWindow& captured) {
    captured.item = createGraphicsCaptureItemForWindow(captured.hwnd);

    if (captured.item == nullptr) {
        captured.attached = false;
        return false;
    }

    const auto size = captured.item.Size();

    captured.sourceSize.cx = size.Width;
    captured.sourceSize.cy = size.Height;

    if (captured.sourceSize.cx <= 0 || captured.sourceSize.cy <= 0) {
        captured.attached = false;
        return false;
    }

    captured.framePool =
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
            winrtDevice,
            winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            size
        );

    if (captured.framePool == nullptr) {
        captured.attached = false;
        return false;
    }

    captured.session = captured.framePool.CreateCaptureSession(captured.item);

    if (captured.session == nullptr) {
        captured.attached = false;
        return false;
    }

    const WindowId id = captured.id;

    captured.frameArrivedToken = captured.framePool.FrameArrived(
        [this, id](
            const winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool&,
            const winrt::Windows::Foundation::IInspectable&
        ) {
            markFrameDirty(id);
        }
    );

    captured.session.StartCapture();
    captured.captureStarted = true;
    captured.frameAvailable = false;
    captured.frameDirty = false;

    return true;
}

void GraphicsCaptureManager::closeCapture(CapturedWindow& captured) {
    if (captured.framePool != nullptr && captured.frameArrivedToken.value != 0) {
        captured.framePool.FrameArrived(captured.frameArrivedToken);
        captured.frameArrivedToken = {};
    }

    if (captured.session != nullptr) {
        captured.session.Close();
        captured.session = nullptr;
    }

    if (captured.framePool != nullptr) {
        captured.framePool.Close();
        captured.framePool = nullptr;
    }

    captured.latestTexture.Reset();
    captured.item = nullptr;

    captured.attached = false;
    captured.captureStarted = false;
    captured.frameAvailable = false;
    captured.frameDirty = false;
    captured.sourceSize = {};
}

void GraphicsCaptureManager::markFrameDirty(WindowId id) {
    dirtyWindows_.insert(id);

    auto it = capturedWindows_.find(id);

    if (it != capturedWindows_.end()) {
        it->second.frameDirty = true;
    }
}

void GraphicsCaptureManager::drainFrames(CapturedWindow& captured) {
    if (!captured.attached || !captured.captureStarted || captured.framePool == nullptr) {
        captured.frameAvailable = false;
        captured.frameDirty = false;
        return;
    }

    bool gotFrame = false;

    while (true) {
        auto frame = captured.framePool.TryGetNextFrame();

        if (frame == nullptr) {
            break;
        }

        auto surface = frame.Surface();

        auto access =
            surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

        const HRESULT result = access->GetInterface(
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void**>(texture.GetAddressOf())
        );

        if (FAILED(result)) {
            continue;
        }

        captured.latestTexture = texture;

        const auto contentSize = frame.ContentSize();
        captured.sourceSize.cx = contentSize.Width;
        captured.sourceSize.cy = contentSize.Height;

        gotFrame = true;
    }

    captured.frameAvailable = gotFrame || captured.latestTexture != nullptr;
    captured.frameDirty = false;
}