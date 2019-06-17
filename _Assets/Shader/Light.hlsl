//PBR - Physically Based Rendering
//-Galobal Illumimation -> IBL -> Image Based Lighting
//-Energy Conservation
//-Reflectivity -> diffuse & spectular
//-Microsurface -> roughness
//-Fresnel's Law
//-Metalicity

#include "Common.hlsl"
#include "BRDF.hlsl"
#include "IBL.hlsl"
#include "SSR.hlsl"

Texture2D albedo_texture        : register(t0);
Texture2D normal_texture        : register(t1);
Texture2D depth_texture         : register(t2);
Texture2D material_texture      : register(t3);
Texture2D shadow_texture        : register(t4);
Texture2D ssao_texture          : register(t5);
Texture2D frame_texture         : register(t6);
Texture2D enviroment_texture    : register(t7);
Texture2D lut_ibl_texture       : register(t8);

SamplerState linear_clamp       : register(s0);
SamplerState point_clamp        : register(s1);

#define MaxLights 64
cbuffer LightBuffer : register(b1)
{
    matrix wvp;
    matrix vp_inv;
    float4 dir_light_color;
    float4 dir_light_intensity;
    float4 dir_light_direction;

    float4 point_light_position[MaxLights];
    float4 point_light_color[MaxLights];
    float4 point_light_intensity_range[MaxLights];

    float4 spot_light_color[MaxLights];
    float4 spot_light_position[MaxLights];
    float4 spot_light_direction[MaxLights];
    float4 spot_light_intensity_range_angle[MaxLights];

    float point_lightcount;
    float spot_lightcount;

    int screen_space_reflection;
}

PixelTexture VS(VertexTexture input)
{
    PixelTexture output;

    output.position = mul(input.position, wvp);
    output.uv       = input.uv;

    return output;
}

float4 PS(PixelTexture input) : SV_Target
{
    float2 texcoord             = input.uv;
    float3 color                = 0.0f;

    float4 albedo               = albedo_texture.Sample(linear_clamp, texcoord);
    float4 normal_sample        = normal_texture.Sample(linear_clamp, texcoord);
    float3 normal               = normalize(normal_sample.xyz);
    float4 material_sample      = material_texture.Sample(linear_clamp, texcoord);

    float occlusion_texture     = normal_sample.w;
    float occlusion_ssao        = ssao_texture.Sample(linear_clamp, texcoord).r;
    float shadow_directional    = shadow_texture.Sample(linear_clamp, texcoord).r;

    Material material;
    material.albedo             = albedo;
    material.roughness          = material_sample.r;
    material.metallic           = material_sample.g;
    material.emission           = material_sample.b;
    material.F0                 = lerp(0.04f, material.albedo, material.metallic);
    material.roughness_alpha    = max(0.001f, material.roughness * material.roughness);

    float depth             = depth_texture.Sample(linear_clamp, texcoord).r;
    float3 world_position   = GetWorldPositionFromDepth(depth, vp_inv, texcoord);
    float3 camera_to_pixel  = normalize(world_position - global_camera_position);

    //Skybox
    if(material_sample.a == 0.0f)
    {
        color = enviroment_texture.Sample(linear_clamp, DirectionToSphereUV(camera_to_pixel)).rgb;
        color *= clamp(dir_light_intensity.r, 0.01f, 1.0f);
        return float4(color, 1.0f);
    }

    //Ambient Light
    float factor_occlusion      = occlusion_texture == 1.0f ? occlusion_ssao : occlusion_texture;
    float factor_self_shadow    = shadow_directional * saturate(dot(normal, normalize(-dir_light_direction).xyz));
    float factor_sky_light      = clamp(dir_light_intensity.r * factor_self_shadow, 0.025f, 1.0f);
    float ambient_light         = factor_sky_light * factor_occlusion;

    //IBL
    color += ImageBasedLighting(material, normal, camera_to_pixel, enviroment_texture, lut_ibl_texture, linear_clamp) * ambient_light;

    //SSR
    if (screen_space_reflection != 0)
    {
        float4 ssr = SSR(world_position, normal, frame_texture, depth_texture, point_clamp);
        color += ssr.xyz * (1.0f - material.roughness) * ambient_light;
    }

    //Emission
    float3 emission = material.emission * albedo.rgb * 40.0f;
    color += emission;

    //Directional Light
    Light directional_light;
    directional_light.color = dir_light_color.rgb;
    directional_light.direction = normalize(-dir_light_direction).xyz;
    
    float directional_micro_shadow = Micro_Shadow(factor_occlusion, normal, directional_light.direction, shadow_directional);
    directional_light.intensity = dir_light_intensity.r * directional_micro_shadow;

    //Compute Illumination
    if(directional_light.intensity > 0.0f)
        color += BRDF(material, directional_light, normal, camera_to_pixel);

    //Point Light


    //Spot Light

    return float4(color, 1.0f);
}