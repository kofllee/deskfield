#pragma once

#include <d3d11_1.h>
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

    ID3D11Device1* device1() const {
        return device1_.Get();
    }

    ID3D11DeviceContext1* context1() const {
        return context1_.Get();
    }

    IDXGIDevice* dxgiDevice() const {
        return dxgiDevice_.Get();
    }

    bool isValid() const {
        return device_ != nullptr && context_ != nullptr;
    }

    bool supportsD3D11_1() const {
        return device1_ != nullptr && context1_ != nullptr;
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};

    Microsoft::WRL::ComPtr<ID3D11Device1> device1_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context1_{};

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice_{};
};