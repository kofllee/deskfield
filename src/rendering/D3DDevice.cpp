#include "D3DDevice.h"

#include <iterator>

bool D3DDevice::initialize() {
    if (isValid()) {
        return true;
    }

    constexpr D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    D3D_FEATURE_LEVEL selectedFeatureLevel{};

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device_.GetAddressOf(),
        &selectedFeatureLevel,
        context_.GetAddressOf()
    );

#if defined(_DEBUG)
    if (FAILED(result)) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;

        result = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            device_.GetAddressOf(),
            &selectedFeatureLevel,
            context_.GetAddressOf()
        );
    }
#endif

    if (FAILED(result)) {
        return false;
    }

    result = device_.As(&dxgiDevice_);

    return SUCCEEDED(result);
}
