#include "GraphicsCaptureInterop.h"

#include <windows.graphics.capture.interop.h>
#include <winrt/base.h>

winrt::Windows::Graphics::Capture::GraphicsCaptureItem createGraphicsCaptureItemForWindow(HWND hwnd) {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        return nullptr;
    }

    auto activationFactory =
        winrt::get_activation_factory<
            winrt::Windows::Graphics::Capture::GraphicsCaptureItem,
            IGraphicsCaptureItemInterop
        >();

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{nullptr};

    const HRESULT result = activationFactory->CreateForWindow(
        hwnd,
        winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(),
        winrt::put_abi(item)
    );

    if (FAILED(result)) {
        return nullptr;
    }

    return item;
}
