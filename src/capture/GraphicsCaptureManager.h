#pragma once

#include "capture/CapturedWindow.h"

#include "workspace/WindowId.h"

#include <d3d11.h>
#include <windows.h>

#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

#include <unordered_map>
#include <unordered_set>
#include <cstdint>

struct CaptureStatus {
    bool attached{};
    bool hasTexture{};
    bool fresh{};
    bool stale{};
    bool failed{};

    std::uint64_t framesReceived{};
    double secondsSinceLastFrame{};

    SIZE sourceSize{};
    SIZE frameContentSize{};
};

class GraphicsCaptureManager {
public:
    bool initialize(ID3D11Device* d3dDevice);
    void shutdown();

    bool attach(WindowId id, HWND hwnd);
    void detach(WindowId id);
    void detachAll();

    void update();

    bool isAttached(WindowId id) const;
    const CapturedWindow* find(WindowId id) const;

    ID3D11Texture2D* latestTexture(WindowId id) const;
    SIZE sourceSize(WindowId id) const;

    CaptureStatus captureStatus(WindowId id) const;

private:
    bool createCaptureForWindow(CapturedWindow& captured);
    void closeCapture(CapturedWindow& captured);

    void markFrameDirty(WindowId id);
    void drainFrames(CapturedWindow& captured);

private:
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice winrtDevice{nullptr};
    std::unordered_map<WindowId, CapturedWindow> capturedWindows_{};

    std::unordered_set<WindowId> dirtyWindows_{};
};