//================================================================
//Box Blur
//================================================================
float4 BoxBlur
(
    float2 uv,
    float2 texel_size,
    int blur_size,
    Texture2D source_texture,
    SamplerState bilinear_sampler
)
{
    return float4(0, 0, 0, 0);
}

//================================================================
//Gaussian Blur
//================================================================
float CalcGaussianWeight(int sampleDist, float sigma)
{
    float g = 1.0f / sqrt(2.0f * 3.141592f * sigma * sigma);
    return (g * exp(-(sampleDist * sampleDist) / (2.0f * sigma * sigma)));
}

float4 GaussianBlur
(
    float2 uv,
    Texture2D srcTexture,
    SamplerState samp,
    float2 texelSize,
    float2 direction,
    float sigma
)
{
    float totalWeights  = 0.0f;
    float4 color        = 0.0f;

    for (int i = -5; i < 5; i++)
    {
        float2 texCoord = uv + (i * texelSize * direction);
        float weight = CalcGaussianWeight(i, sigma);

        color += srcTexture.SampleLevel(samp, texCoord, 0) * weight;
        totalWeights += weight;
    }

    color /= totalWeights;
    return color;
}

//================================================================
//Gaussian Bilateral Blur
//================================================================
float4 GaussianBilateralBlur
(
    float2 uv,
    Texture2D source_texture,
    Texture2D depth_texture,
    Texture2D normal_texture,
    SamplerState bilinear_sampler,
    float2 texel_size,
    float2 direction,
    float sigma
)
{
    float weight_sum = 0.0f;
    float4 color = 0.0f;
    float center_depth = depth_texture.SampleLevel(bilinear_sampler, uv, 0).r;
    float3 center_normal = normalize(normal_texture.SampleLevel(bilinear_sampler, uv, 0).xyz);
    float threshold = 0.0005f;

    for (int i = -5; i < 5; i++)
    {
        float2 texcoord         = uv + (i * texel_size * direction);
        float sample_depth      = depth_texture.SampleLevel(bilinear_sampler, texcoord, 0).r;
        float3 sample_normal    = normalize(normal_texture.SampleLevel(bilinear_sampler, texcoord, 0).xyz);

        float awareness_depth   = saturate(threshold - abs(center_depth - sample_depth));
        float awareness_normal  = saturate(dot(center_normal, sample_normal));
        float awareness         = awareness_depth * awareness_normal;

        float weight            = CalcGaussianWeight(i, sigma) * awareness;
        color                   += source_texture.SampleLevel(bilinear_sampler, texcoord, 0) * weight;
        weight_sum              += weight;
    }
    color /= weight_sum;

    return color;
}