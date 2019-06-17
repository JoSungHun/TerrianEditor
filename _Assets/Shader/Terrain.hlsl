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
    float padding;
}

cbuffer Actor : register(b2)
{
    matrix model_world;
    matrix wvp_current;
    matrix wvp_previous;
}

cbuffer Terrain : register(b3)
{
    float3 brush_position : packoffset(c0);
    float brush_range : packoffset(c0.w);
    float is_picked : packoffset(c1);
    float terrain_width : packoffset(c1.y);
    float terrain_height : packoffset(c1.z);
    float tess_factor : packoffset(c1.w);
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
    float4 BrushColor           : COLOR0;
};

struct PixelOutput
{
    float4 albedo   : SV_Target0;
    float4 normal   : SV_Target1;
    float4 material : SV_Target2;
    float2 velocity : SV_Target3;
};

Texture2D splat_Texture             : register(t0);
Texture2D brush_texture             : register(t1);
Texture2D channel_default_diffuse   : register(t2);
Texture2D channel_default_normal    : register(t3);
Texture2D channel_1_diffuse         : register(t4);
Texture2D channel_1_normal          : register(t5);
Texture2D channel_2_diffuse         : register(t6);
Texture2D channel_2_normal          : register(t7);
Texture2D channel_3_diffuse         : register(t8);
Texture2D channel_3_normal          : register(t9);
Texture2D channel_4_diffuse         : register(t10);
Texture2D channel_4_normal          : register(t11);

SamplerState samp : register(s0);

float Brush(float3 Position)
{
    float brushW = 0, brushH = 0;
    brush_texture.GetDimensions(brushW, brushH);
    int minX, maxX, minZ, maxZ;
    minX = brush_position.x - brush_range;

    if (minX <= 0)
        minX = 0;

    maxX = brush_position.x + brush_range;

    if (maxX >= terrain_width)

        maxX = terrain_width;
    
    minZ = brush_position.z - brush_range;

    if (minZ <= 0)
        minZ = 0;
    maxZ = brush_position.z + brush_range;

    if (maxZ >= terrain_height)

        maxZ = terrain_height;

    if (Position.x < minX || Position.x > maxX

        || Position.z < minZ || Position.z > maxZ)

        return -1;

    float2 uvBrush = float2((Position.x - (brush_position.x - brush_range)) / (2 * brush_range), (Position.z - (brush_position.z - brush_range)) / (2 * brush_range));
    float4 color = brush_texture.Sample(samp, uvBrush);
    if (color.g < 0.01f)
        return -1;

    return color.g;

}


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

    float4 channelValue  = splat_Texture.Sample(samp,input.uv);
    float defaultValue = 1.0f - channelValue.r - channelValue.g - channelValue.b - channelValue.a;
    
    float3 brushPos = dot(input.positionWS.xyz, brush_position);

    

    
#if ALBEDO_TEXTURE
    
    float4 brush = float4(0,Brush(input.positionWS),0,0);

        albedo = channel_default_diffuse.Sample(samp,textcoord)*defaultValue;
        albedo += channel_1_diffuse.Sample(samp,textcoord)* channelValue.r;
        albedo += channel_2_diffuse.Sample(samp,textcoord)* channelValue.g;
        albedo += channel_3_diffuse.Sample(samp,textcoord)* channelValue.b;
        albedo += channel_4_diffuse.Sample(samp,textcoord)* channelValue.a;
        albedo *= material_albedo_color;
        if(brush.g > 0 && is_picked>0)
            albedo += brush;
    
    

    
#endif

#if NORMAL_TEXTURE
    // NormalMap blending
    //

    float3x3 TBN = MakeTBN(input.normal, input.tangent);
    
    // float3 normalSample = normalize(Unpack(normal_texture.Sample(samp, textcoord).rgb));
    // normalIntensity = clamp(normalIntensity, 0.01f, 1.0f);
    // normalSample.x *= normalIntensity;
    // normalSample.y *= normalIntensity;
    
    //normal = normalize(mul(normalSample, TBN));
#endif

    gbuffer.albedo      = albedo;
    gbuffer.normal      = float4(normalize(normal), occlusion);
    gbuffer.material    = float4(roughness, metallic, emission, material_shading_mode);
    gbuffer.velocity    = velocity;
   
    return gbuffer;
}
