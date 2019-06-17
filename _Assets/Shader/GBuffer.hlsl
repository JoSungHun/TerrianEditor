#include "Common.hlsl"

cbuffer MaterialBuffer : register(b1)
{
    float4 material_albedo_color;
    float2 material_tiling;
    float2 material_offset;
    float material_roughness;
    float material_metallic;
    float material_normal_strength;
    float material_height;
    float material_shading_mode;
}

cbuffer ActorBuffer : register(b2)
{
    matrix model_world;
    matrix wvp_current;
    matrix wvp_previous;
}

struct PixelInput
{
    float4 positionCS           : SV_POSITION0;
    float2 uv                   : TEXCOORD0;
    float3 normal               : NORMAL0;
    float3 tangent              : TANGENT0;
    float4 positionVS           : POSITION0;
    float4 positionWS           : POSITION1;
    float4 positionCS_current   : SCREEN_POSITION_CUR0;
    float4 positionCS_previouse : SCREEN_POSITION_PREV0;
};

struct PixelOutput
{
    float4 albedo   : SV_Target0;
    float4 normal   : SV_Target1;
    float4 material : SV_Target2;
    float2 velocity : SV_Target3;
};

PixelInput VS(VertexTNTBB input)
{
    PixelInput output;

    output.positionWS           = mul(input.position, model_world);
    output.positionVS           = mul(output.positionWS, global_view);
    output.positionCS           = mul(output.positionVS, global_proj);
    output.positionCS_current   = mul(input.position, wvp_current);
    output.positionCS_previouse = mul(input.position, wvp_previous);
    output.normal               = normalize(mul(input.normal, (float3x3) model_world));
    output.tangent              = normalize(mul(input.tangent, (float3x3) model_world));
    output.uv                   = input.uv;

    return output;
}


Texture2D albedo_texture    : register(t0);
Texture2D roughness_texture : register(t1);
Texture2D metallic_texture  : register(t2);
Texture2D normal_texture    : register(t3);
Texture2D height_texture    : register(t4);
Texture2D occlusion_texture : register(t5);
Texture2D emission_texture  : register(t6);
Texture2D mask_texture      : register(t7);

SamplerState samp : register(s0);

PixelOutput PS(PixelInput input)
{
    PixelOutput gbuffer;

    float4 albedo               = material_albedo_color;
    float roughness             = abs(material_roughness);
    float metallic              = saturate(material_metallic);
    float3 normal               = input.normal;
    float normalIntensity       = clamp(material_normal_strength, 0.012f, material_normal_strength);
    float emission              = 0.0f;
    float occlusion             = 1.0f;

    float2 position_current     = input.positionCS_current.xy / input.positionCS_current.w;
    float2 position_previous    = input.positionCS_previouse.xy / input.positionCS_previouse.w;
    float2 position_delta       = position_current - position_previous;
    float2 velocity             = position_delta * float2(0.5f, -0.5f);

    float2 textcoord = float2(input.uv.x * material_tiling.x + material_offset.x, input.uv.y * material_tiling.y + material_offset.y);

#if HEIGHT_TEXTURE
#endif

#if MASK_TEXTURE
#endif

#if ALBEDO_TEXTURE
    albedo *= albedo_texture.Sample(samp, textcoord);
#endif

#if ROUGHNESS_TEXTURE
    roughness *= roughness_texture.Sample(samp, textcoord).r;
#endif

#if METALLIC_TEXTURE
    metallic *= metallic_texture.Sample(samp, textcoord).r;
#endif

#if NORMAL_TEXTURE
    float3x3 TBN = MakeTBN(input.normal, input.tangent);
    
    float3 normalSample = normalize(Unpack(normal_texture.Sample(samp, textcoord).rgb));
    normalIntensity = clamp(normalIntensity, 0.01f, 1.0f);
    normalSample.x *= normalIntensity;
    normalSample.y *= normalIntensity;
    
    //normal = normalize(mul(normalSample, TBN));
#endif

#if OCCLUSION_TEXTURE
#endif

#if EMISSION_TEXTURE
#endif

    gbuffer.albedo      = albedo;
    gbuffer.normal      = float4(normalize(normal), occlusion);
    gbuffer.material    = float4(roughness, metallic, emission, material_shading_mode);
    gbuffer.velocity    = velocity;
   
    return gbuffer;
}
