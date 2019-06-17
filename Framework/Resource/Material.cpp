#include "Framework.h"
#include "Material.h"
#include "../Shader/Shader_Standard.h"

Material::Material(Context * context)
    : IResource(context)
    , shading_mode(ShadingMode::PBR)
    , cull_mode(CullMode::BACK)
    , colorAlbedo(0xFFFFFFFF)
    , roughnessMultiplier(1.0f)
    , metallicMultiplier(0.0f)
    , normalMultiplier(0.0f)
    , heightMultiplier(0.0f)
    , uvTiling(1.0f, 1.0f)
    , uvOffset(0.0f, 0.0f)
{
    ZeroMemory(&cpu_material_buffer, sizeof(MaterialData));

    SetStandardShader();
}

Material::~Material()
{
}

void Material::SaveToFile(const std::string & path)
{
}

void Material::LoadFromFile(const std::string & path)
{
}

void Material::SetStandardShader()
{
    ulong flags = 0UL;

    if (HasTexture(TextureType::Albedo))    flags |= ShaderFlags_Albedo;
    if (HasTexture(TextureType::Roughness)) flags |= ShaderFlags_Roughness;
    if (HasTexture(TextureType::Metallic))  flags |= ShaderFlags_Metallic;
    if (HasTexture(TextureType::Normal))    flags |= ShaderFlags_Normal;
    if (HasTexture(TextureType::Height))    flags |= ShaderFlags_Height;
    if (HasTexture(TextureType::Occlusion)) flags |= ShaderFlags_Occlusion;
    if (HasTexture(TextureType::Emissive))  flags |= ShaderFlags_Emissive;
    if (HasTexture(TextureType::Mask))      flags |= ShaderFlags_Mask;

    shader = Shader_Standard::GetMatchingStandardShader(context, flags);
}

auto Material::GetTexture(const TextureType & type) -> Texture *
{
    if (!textures.count(type))
        return nullptr;

    return textures[type];
}

auto Material::GetTextureShaderResource(const TextureType & type) -> ID3D11ShaderResourceView *
{
    auto texture = GetTexture(type);
    return texture ? texture->GetShaderResourceView() : nullptr;
}

void Material::SetTexture(const TextureType & type, Texture * texture)
{
    if (!texture)
        return;

    textures[type] = texture;
    SetStandardShader();
}

void Material::SetTexture(const TextureType & type, const std::string & path)
{
    auto texture = resourceManager->Load<Texture>(path);

    SetTexture(type, texture);
}

auto Material::HasTexture(const TextureType & type) -> const bool
{
    return textures.find(type) != textures.end();
}

auto Material::HasTexture(const std::string & path) -> const bool
{
    for (const auto& texture : textures)
    {
        if (texture.second->GetResourcePath() == path)
            return true;
    }
    return false;
}

void Material::UpdateConstantBuffer()
{
    if (!gpu_material_buffer)
    {
        gpu_material_buffer = std::make_shared<ConstantBuffer>(context);
        gpu_material_buffer->Create<MaterialData>();
    }

    auto check = false;
    check |= cpu_material_buffer.MaterialAlbedoColor    != colorAlbedo                      ? true : false;
    check |= cpu_material_buffer.MaterialTiling         != uvTiling                         ? true : false;
    check |= cpu_material_buffer.MaterialOffset         != uvOffset                         ? true : false;
    check |= cpu_material_buffer.MaterialRoughness      != roughnessMultiplier              ? true : false;
    check |= cpu_material_buffer.MaterialMetallic       != metallicMultiplier               ? true : false;
    check |= cpu_material_buffer.MaterialNormalStrength != normalMultiplier                 ? true : false;
    check |= cpu_material_buffer.MaterialHeight         != heightMultiplier                 ? true : false;
    check |= cpu_material_buffer.MaterialShadingMode    != static_cast<float>(shading_mode) ? true : false;

    if (!check)
        return;

    auto data = gpu_material_buffer->Map<MaterialData>();
    if (!data)
    {
        LOG_ERROR("Invalid to buffer");
        return;
    }

    data->MaterialAlbedoColor    = cpu_material_buffer.MaterialAlbedoColor      = colorAlbedo;
    data->MaterialTiling         = cpu_material_buffer.MaterialTiling           = uvTiling;
    data->MaterialOffset         = cpu_material_buffer.MaterialOffset           = uvOffset;
    data->MaterialRoughness      = cpu_material_buffer.MaterialRoughness        = roughnessMultiplier;
    data->MaterialMetallic       = cpu_material_buffer.MaterialMetallic         = metallicMultiplier;
    data->MaterialNormalStrength = cpu_material_buffer.MaterialNormalStrength   = normalMultiplier;
    data->MaterialHeight         = cpu_material_buffer.MaterialHeight           = heightMultiplier;
    data->MaterialShadingMode    = cpu_material_buffer.MaterialShadingMode      = static_cast<float>(shading_mode);

    gpu_material_buffer->Unmap();
}
