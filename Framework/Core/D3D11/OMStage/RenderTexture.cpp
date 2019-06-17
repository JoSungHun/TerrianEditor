#include "Framework.h"
#include "RenderTexture.h"

RenderTexture::RenderTexture(Context * context)
    : srv(nullptr)
    , dsv(nullptr)
    , format(DXGI_FORMAT_R8G8B8A8_UNORM)
    , array_size(1)
{
    graphics = context->GetSubsystem<Graphics>();

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = 0.0f;
    viewport.Height = 0.0f;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

RenderTexture::~RenderTexture()
{
    Clear();
}

void RenderTexture::Create(const uint & width, const uint & height, const bool & depth_enable, const DXGI_FORMAT & textureFormat, const DXGI_FORMAT & depthFormat, const uint & array_size, const bool & bCube_map)
{
    if (width == 0 || height == 0)
    {
        LOG_ERROR("Invalid parameter");
        return;
    }

    this->format = textureFormat;
    this->array_size = array_size;
    this->depth_enable = depth_enable;
    this->viewport.TopLeftX = 0.0f;
    this->viewport.TopLeftY = 0.0f;
    this->viewport.Width = static_cast<float>(width);
    this->viewport.Height = static_cast<float>(height);
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;

    ID3D11Texture2D* texture = nullptr;

    if (depth_enable)
    {
        //Create DepthStencil Texture
        {
            D3D11_TEXTURE2D_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = array_size;
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            auto hr = graphics->GetDevice()->CreateTexture2D
            (
                &desc,
                nullptr,
                &texture
            );
            assert(SUCCEEDED(hr));
        }

        //Create DepthStencilView
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
            desc.Format = DXGI_FORMAT_D32_FLOAT;
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;

            auto hr = graphics->GetDevice()->CreateDepthStencilView
            (
                texture,
                &desc,
                &dsv
            );
            assert(SUCCEEDED(hr));
        }

        //Create ShaderResourceView
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            desc.Format = DXGI_FORMAT_R32_FLOAT;

            if (array_size == 1)
            {
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipLevels = 1;
                desc.Texture2D.MostDetailedMip = 0;
            }
            else
            {
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.FirstArraySlice = 0;
                desc.Texture2DArray.MostDetailedMip = 0;
                desc.Texture2DArray.MipLevels = 1;
                desc.Texture2DArray.ArraySize = array_size;
            }

            auto hr = graphics->GetDevice()->CreateShaderResourceView
            (
                texture,
                &desc,
                &srv
            );
            assert(SUCCEEDED(hr));
        }
    }
    else
    {
        //Create RenderTarget Texture
        {
            D3D11_TEXTURE2D_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = array_size;
            desc.Format = textureFormat;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = bCube_map ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

            auto hr = graphics->GetDevice()->CreateTexture2D
            (
                &desc,
                nullptr,
                &texture
            );
            assert(SUCCEEDED(hr));
        }

        //Create RenderTargetView
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
            desc.Format = textureFormat;

            if (array_size == 1)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = 0;

                auto& rtv = rtvs.emplace_back(nullptr);
                auto hr = graphics->GetDevice()->CreateRenderTargetView
                (
                    texture,
                    &desc,
                    &rtv
                );
                assert(SUCCEEDED(hr));
            }
            else
            {
                for (uint i = 0; i < array_size; i++)
                {
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    desc.Texture2DArray.MipSlice = 0;
                    desc.Texture2DArray.ArraySize = 1;
                    desc.Texture2DArray.FirstArraySlice = 1;

                    auto& rtv = rtvs.emplace_back(nullptr);
                    auto hr = graphics->GetDevice()->CreateRenderTargetView
                    (
                        texture,
                        &desc,
                        &rtv
                    );
                    assert(SUCCEEDED(hr));
                }
            }
        }

        //Create ShaderResourceView
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
            ZeroMemory(&desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
            desc.Format = textureFormat;

            if (array_size == 1)
            {
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipLevels = 1;
                desc.Texture2D.MostDetailedMip = 0;
            }
            else
            {
                desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.FirstArraySlice = 0;
                desc.Texture2DArray.MostDetailedMip = 0;
                desc.Texture2DArray.MipLevels = 1;
                desc.Texture2DArray.ArraySize = array_size;
            }

            auto hr = graphics->GetDevice()->CreateShaderResourceView
            (
                texture,
                &desc,
                &srv
            );
            assert(SUCCEEDED(hr));
        }
    }

    SAFE_RELEASE(texture);
}

void RenderTexture::Clear()
{
    SAFE_RELEASE(dsv);
    SAFE_RELEASE(srv);

    for (auto& rtv : rtvs)
        SAFE_RELEASE(rtv);
}

void RenderTexture::SetRenderTarget()
{
    //graphics->GetDeviceContext()->RSSetViewports(1, &viewport);
    //graphics->GetDeviceContext()->OMSetRenderTargets(1, &rtv, dsv);
}

void RenderTexture::ClearRenderTarget(const D3DXCOLOR & color)
{
    //graphics->GetDeviceContext()->ClearRenderTargetView(rtv, color);
    //graphics->GetDeviceContext()->ClearDepthStencilView
    //(
    //    dsv, 
    //    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
    //    1.0f,
    //    0
    //);
}