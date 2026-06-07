#include "Direct3DWinRtInterop.h"

#include <dxgi.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/base.h>
#include <wrl/client.h>

winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice createDirect3DDeviceFromD3D11Device(
    ID3D11Device* device
) {
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;

    const HRESULT queryResult = device->QueryInterface(
        __uuidof(IDXGIDevice),
        reinterpret_cast<void**>(dxgiDevice.GetAddressOf())
    );

    if (FAILED(queryResult)) {
        return nullptr;
    }

    winrt::com_ptr<IInspectable> inspectable;

    const HRESULT createResult = CreateDirect3D11DeviceFromDXGIDevice(
        dxgiDevice.Get(),
        inspectable.put()
    );

    if (FAILED(createResult)) {
        return nullptr;
    }

    return inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}