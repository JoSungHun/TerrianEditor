#include "Common.hlsl"

Texture2D depth_texture         : register(t0);
Texture2D enviroment_texture    : register(t1);

SamplerState linear_clamp       : register(s0);

cbuffer TransparentBuffer : register(b1)
{
    matrix world;
    matrix wvp;
    float4 color;
    float3 camera_pos;
    float roughness;
    float3 light_dir;
};

struct PixelInputType
{
    float4 position         : SV_POSITION0;
    float2 uv               : TEXCOORD0;
    float3 normal           : NORMAL0;
    float3 tangent          : TANGENT0;
    float4 positionWS       : POSITION0;
};

PixelInputType VS(VertexTNTBB input)
{
    PixelInputType output;

    output.position         = mul(input.position, wvp);
    output.positionWS       = mul(input.position, world);
    output.uv               = input.uv;
    output.normal           = normalize(mul(input.normal, (float3x3) world));
    output.tangent          = normalize(mul(input.tangent, (float3x3) world));

    return output;
}

float4 PS(PixelInputType input) : SV_Target
{
    float3 camera_to_pixel      = input.positionWS.xyz - camera_pos;
    float transparent_distance  = length(camera_to_pixel);
    camera_to_pixel             = normalize(camera_to_pixel);
    float opaque_distance       = depth_texture.Sample(linear_clamp, Project(input.position));
    
    if(opaque_distance > transparent_distance)
        discard;

    float3 normal               = normalize(input.normal);
    float3 reflection           = reflect(camera_to_pixel, normal);
    float3 environment_color    = enviroment_texture.Sample(linear_clamp, DirectionToSphereUV(reflection));

    float specular_hardness     = 0;
    float3 H                    = normalize(light_dir - camera_to_pixel);
    float NdotH                 = dot(normal, H);
    float intensity             = pow(saturate(NdotH), specular_hardness);

    float alpha                 = color.a;
    float3 final_color          = saturate(color.rgb * intensity + environment_color);

    return float4(final_color,alpha);
}