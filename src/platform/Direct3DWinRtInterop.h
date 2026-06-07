#pragma once

#include <d3d11.h>

#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice createDirect3DDeviceFromD3D11Device(
    ID3D11Device* device
);