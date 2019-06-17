#pragma once
#include "../IStage.h"

class RenderTexture final : public IStage
{
public:
    RenderTexture(class Context* context);
    ~RenderTexture();

    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    auto GetRenderTargetView(const uint& index = 0) -> ID3D11RenderTargetView* const { return index < rtvs.size() ? rtvs[index] : nullptr;; }
    auto GetDepthStencilView() -> ID3D11DepthStencilView* const { return dsv; }
    auto GetShaderResourceView()->ID3D11ShaderResourceView* const { return srv; }

    auto GetViewport() const -> const D3D11_VIEWPORT& { return viewport; }
    auto GetFormat() const -> const DXGI_FORMAT& { return format; }
    auto GetWidth() const -> const uint { return static_cast<uint>(viewport.Width); }
    auto GetHeight() const -> const uint { return static_cast<uint>(viewport.Height); }
    auto GetArraySize() const -> const uint& { return array_size; }

    void Create
    (
        const uint& width,
        const uint& height,
        const bool& depth_enable = false,
        const DXGI_FORMAT& textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
        const DXGI_FORMAT& depthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
        const uint& array_size = 1,
        const bool& bCube_map = false
    );
    void Clear();

    void SetRenderTarget();
    void ClearRenderTarget(const D3DXCOLOR& color = 0xff555566);

private:
    class Graphics* graphics;

    std::vector<ID3D11RenderTargetView*> rtvs;
    ID3D11ShaderResourceView* srv;
    ID3D11DepthStencilView* dsv;
    D3D11_VIEWPORT viewport;
    DXGI_FORMAT format;
    uint array_size;
    bool depth_enable;
};