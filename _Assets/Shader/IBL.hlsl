static const float MaxMipLevel = 11.0f;
static const float2 EnvironmentMipSize[12] =
{
    float2(4096, 2048),
    float2(2048, 1024),
    float2(1024, 512),
    float2(512, 256),
    float2(256, 128),
    float2(128, 64),
    float2(64, 32),
    float2(32, 16),
    float2(16, 8),
    float2(8, 4),
    float2(4, 2),
    float2(2, 1),
};

float3 SampleEnvironment(SamplerState linear_sampler, Texture2D environment_texture, float2 uv, float mip_level)
{
    float2 texel_size   = EnvironmentMipSize[mip_level];
    float dx            = 1.0f * texel_size.x;
    float dy            = 1.0f * texel_size.y;

    // top left , top right, bottom left, bottom right, center
    float3 tl           = environment_texture.SampleLevel(linear_sampler, uv + float2(-dx, -dy), mip_level).rgb;
    float3 tr           = environment_texture.SampleLevel(linear_sampler, uv + float2(+dx, -dy), mip_level).rgb;
    float3 bl           = environment_texture.SampleLevel(linear_sampler, uv + float2(-dx, +dy), mip_level).rgb;
    float3 br           = environment_texture.SampleLevel(linear_sampler, uv + float2(+dx, +dy), mip_level).rgb;
    float3 ce           = environment_texture.SampleLevel(linear_sampler, uv, mip_level).rgb;

    return (tl + tr + bl + br + ce) / 5.0f;
}

float3 GetSpecularDominantDirection(float3 normal, float3 reflection, float roughness)
{
    const float smoothness  = 1.0f - roughness; // roughness 0~1
    const float factor      = smoothness * (sqrt(smoothness) + roughness);
    // normal? reflection vector? smooth roughness ??? ?? ??
    return lerp(normal, reflection, factor);
}

float3 FresnelSchlickRoughness(float cos_theta, float3 F0, float roughness)
{
    float smoothness = 1.0f - roughness;
    return F0 + (max(float3(smoothness, smoothness, smoothness), F0) - F0) * pow(1.0f - cos_theta, 5.0f);

}

float3 ImageBasedLighting
(
    Material material, 
    float3 normal, 
    float3 camera_to_pixel, 
    Texture2D environment_texture, 
    Texture2D lut_ibl_texture, 
    SamplerState linear_sampler
)
{
    float3 reflection       = reflect(camera_to_pixel, normal);
    reflection              = GetSpecularDominantDirection(normal, reflection, material.roughness);

    float NdotV             = saturate(dot(-camera_to_pixel, normal));
    float3 F                = FresnelSchlickRoughness(NdotV, material.F0, material.roughness);

    float3 kS               = F; // ???? ?? ???
    float3 kD               = 1.0f - kS; // ?? ???, ???? ?
    kD                      *= 1.0f - material.metallic; // ?? ??? ?? ? ??? ? ????

    float3 irradiance       = SampleEnvironment(linear_sampler, environment_texture, DirectionToSphereUV(normal), 8);
    float3 diffuse          = irradiance * material.albedo; // ? albedo ?? ??? ?? ?? ?????.

    float mip_level         = material.roughness_alpha * MaxMipLevel;
    float3 pre_filter_color = SampleEnvironment(linear_sampler, environment_texture, DirectionToSphereUV(reflection), mip_level);
    float2 environment_BRDF = lut_ibl_texture.Sample(linear_sampler, float2(NdotV, material.roughness)).xy;
    float3 specular         = pre_filter_color * (F * environment_BRDF.x + environment_BRDF.y);

    return kD * diffuse + specular;
}