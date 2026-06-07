#pragma once

#include "workspace/WindowId.h"

#include <d3d11.h>
#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

#include <wrl/client.h>

struct CapturedWindow {
    WindowId id{};
    HWND hwnd{};

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool framePool{nullptr};
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{nullptr};

    winrt::event_token frameArrivedToken{};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> latestTexture{};
    SIZE sourceSize{};

    bool attached{};
    bool frameAvailable{};
    bool frameDirty{};
    bool captureStarted{};
};