#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

class D3DDevice {
public:
    D3DDevice() = default;
    ~D3DDevice() = default;

    D3DDevice(const D3DDevice&) = delete;
    D3DDevice& operator=(const D3DDevice&) = delete;

    bool initialize();

    ID3D11Device* device() const {
        return device_.Get();
    }

    ID3D11DeviceContext* context() const {
        return context_.Get();
    }

    IDXGIDevice* dxgiDevice() const {
        return dxgiDevice_.Get();
    }

    bool isValid() const {
        return device_ != nullptr && context_ != nullptr;
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice_{};
};

