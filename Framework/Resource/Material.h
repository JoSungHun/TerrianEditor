#pragma once
#include "IResource.h"

enum class TextureType : uint
{
    Unknown,
    Albedo,
    Roughness,
    Metallic,
    Normal,
    Height,
    Occlusion,
    Emissive,
    Mask,
};

enum class ShadingMode : uint
{
    Sky, PBR
};

class Material final : public IResource
{
public:
    Material(class Context* context);
    ~Material();

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    void SaveToFile(const std::string& path) override;
    void LoadFromFile(const std::string& path) override;

    auto GetShader()->class Shader* const { return shader; }
    void SetShader(class Shader* shader) { this->shader = shader; }
    void SetStandardShader();

    auto GetTexture(const TextureType& type) -> Texture*;
    auto GetTextureShaderResource(const TextureType& type) -> ID3D11ShaderResourceView*;
    void SetTexture(const TextureType& type, Texture* texture);
    void SetTexture(const TextureType& type, const std::string& path);

    auto HasTexture(const TextureType& type)->const bool;
    auto HasTexture(const std::string& path)->const bool;

    auto GetShadingMode() const -> const ShadingMode& { return shading_mode; }
    void SetShadingMode(const ShadingMode& shading_mode) { this->shading_mode = shading_mode; }

    auto GetCullMode() const -> const CullMode& { return cull_mode; }
    void SetCullMode(const CullMode& cull_mode) { this->cull_mode = cull_mode; }

    auto GetRoughnessMultiplier() const -> const float& { return roughnessMultiplier; }
    void SetRoughnessMultiplier(const float& roughnessMultiplier) { this->roughnessMultiplier = roughnessMultiplier; }
   
    auto GetMetallicMultiplier() const -> const float& { return metallicMultiplier; }
    void SetMetallicMultiplier(const float& metallicMultiplier) { this->metallicMultiplier = metallicMultiplier; }

    auto GetNormalMultiplier() const -> const float& { return normalMultiplier; }
    void SetNormalMultiplier(const float& normalMultiplier) { this->normalMultiplier = normalMultiplier; }

    auto GetHeightMultiplier() const ->const float& { return heightMultiplier; }
    void SetHeightMultiplier(const float& heightMultiplier) { this->heightMultiplier = heightMultiplier; }

    auto GetColorAlbedo() const ->const D3DXCOLOR& { return colorAlbedo; }
    void SetColorAlbedo(const D3DXCOLOR& colorAlbedo) { this->colorAlbedo = colorAlbedo; }
    
    auto GetTiling() const ->const D3DXVECTOR2& { return uvTiling; }
    void SetTiling(const D3DXVECTOR2& uvTiling) { this->uvTiling = uvTiling; }

    auto GetOffset() const ->const D3DXVECTOR2& { return uvOffset; }
    void SetOffset(const D3DXVECTOR2& uvOffset) { this->uvOffset = uvOffset; }

    auto GetConstantBuffer() const -> class ConstantBuffer* { return gpu_material_buffer.get(); }
    void UpdateConstantBuffer();

private:
    class Shader* shader;

    MaterialData cpu_material_buffer;
    std::shared_ptr<class ConstantBuffer> gpu_material_buffer;

    ShadingMode shading_mode;
    CullMode cull_mode;
    D3DXCOLOR colorAlbedo;
    float roughnessMultiplier;
    float metallicMultiplier;
    float normalMultiplier;
    float heightMultiplier;
    D3DXVECTOR2 uvTiling;
    D3DXVECTOR2 uvOffset;

    std::map<TextureType, Texture*> textures;
};