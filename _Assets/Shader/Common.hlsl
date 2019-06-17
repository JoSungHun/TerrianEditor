#include "CommonVertex.hlsl"
#include "CommonBuffer.hlsl"

//================================================================
//Global Constant
//================================================================
#define PI 3.1415926535897932384626433832795
#define INV_PI 1.0 / PI
#define EPSILON 0.00000001

//================================================================
//Struct
//================================================================
struct Material
{
    float3 albedo;
    float roughness;
    float metallic;
    float emission;
    float3 F0;
    float roughness_alpha;
};

struct Light
{
    float3 color;
    float intensity;
    float3 direction;
};
//================================================================
//Gamma
//================================================================
float4 Degamma(float4 color)
{
    return pow(abs(color), 2.2f);
}

float3 Degamma(float3 color)
{
    return pow(abs(color), 2.2f);
}

float4 Gamma(float4 color)
{
    return pow(abs(color), 1.0f/2.2f);
}

float3 Gamma(float3 color)
{
    return pow(abs(color), 1.0f/2.2f);
}

//================================================================
//Packing
//================================================================
float3 Pack(float3 value)//[-1, 1] -> [0, 1]
{
    return value * 0.5f + 0.5f;
}

float3 Unpack(float3 value)//[0, 1] -> [-1, 1]
{
    return value * 2.0f - 1.0f;
}

//================================================================
//Normal
//================================================================
float3x3 MakeTBN(float3 n, float3 t)
{
    float3 b = cross(n, t);
    return float3x3(t, b, n);
}

//================================================================
//Luminance
//================================================================
float Luminance(float3 color)
{
    return max(dot(color, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

float Luminance(float4 color)
{
    return max(dot(color.rgb, float3(0.299f, 0.587f, 0.114f)), 0.0001f);
}

//================================================================
//Depth
//================================================================
float GetDepth(Texture2D depth_texture, SamplerState linear_sampler, float2 uv)
{
    return depth_texture.SampleLevel(linear_sampler, uv, 0).r;
}

float GetLinearDepth(float depth)
{
    float z = 2.0f * depth - 1.0f;
    return 2.0f * global_camera_far * global_camera_near / (global_camera_near + global_camera_far - z * (global_camera_near - global_camera_far));

}

float GetLinearDepth(Texture2D depth_texture, SamplerState linear_sampler, float2 uv)
{
    float depth = GetDepth(depth_texture, linear_sampler, uv);
    return GetLinearDepth(depth);
}

float3 GetWorldPositionFromDepth(float z, matrix vp_inverse, float2 uv)
{
    float x                 = uv.x * 2.0f - 1.0f;
    float y                 = (1.0f - uv.y) * 2.0f - 1.0f;
    float4 position_clip    = float4(x, y, z, 1.0f);
    float4 position_world   = mul(position_clip, vp_inverse);

    return position_world.xyz / position_world.w;
}

float3 GetWorldPositionFromDepth(Texture2D depth_texture, SamplerState linear_sampler, float2 uv)
{
    float depth = GetDepth(depth_texture, linear_sampler, uv);
    return GetWorldPositionFromDepth(depth, global_vp_inverse, uv);
}

float3 GetWorldPositionFromDepth(float depth, float2 uv)
{
    return GetWorldPositionFromDepth(depth, global_vp_inverse, uv);
}

//================================================================
//Sky Sphere Uv
//================================================================
float2 DirectionToSphereUV(float3 direction)
{
    float n     = length(direction.xz);
    float2 uv   = float2((n > EPSILON) ? direction.x / n : 0.0f, direction.y);

    uv          = acos(uv) * INV_PI;
    uv.x        = (direction.z > 0.0f) ? uv.x * 0.5f : 1.0f - (uv.x * 0.5f);

    return uv;
}

//================================================================
//Project
//================================================================
float2 Project(float4 value)
{
    return (value.xy / value.w) * float2(0.5f, -0.5f) + 0.5f;
}

float2 Project(float3 position, matrix transform)
{
    float4 proj_coord = mul(float4(position, 1.0f), transform);
    proj_coord.xy /= proj_coord.w;
    proj_coord.xy = proj_coord.xy * float2(0.5f, -0.5f) + 0.5f;

    return proj_coord.xy;
}

//================================================================
//MISC
//================================================================
//The Technical Art of Unchared 4 - http://advances.realtimerendering.com/other/2016/naughty_dog/index.html
float Micro_Shadow(float ao, float3 N, float3 L, float shadow)
{
    float aperture = 2.0f * ao * ao;
    float micro_shadow = saturate(abs(dot(L, N)) + aperture - 1.0f);
    return shadow * micro_shadow;
}