//================================================================
//Downsample with a 4x4 box filter
//================================================================
float4 DownsampleBox
(
    float2 uv,
    float2 texel_size,
    Texture2D source_texture,
    SamplerState bilinear_sampler
)
{
    float4 d = texel_size.xyxy * float4(-1.0f, -1.0f, 1.0f, 1.0f);

    float4 s;
    s  = source_texture.Sample(bilinear_sampler, uv + d.xy);
    s += source_texture.Sample(bilinear_sampler, uv + d.zy);
    s += source_texture.Sample(bilinear_sampler, uv + d.xw);
    s += source_texture.Sample(bilinear_sampler, uv + d.zw);

    return s * (1.0f / 4.0f);
}

//================================================================
//Downsample with a 4x4 box filter + anti-flicker filter
//================================================================
float4 DownsampleBox_AntiFlicker
(
    float2 uv,
    float2 texel_size,
    Texture2D source_texture,
    SamplerState bilinear_sampler
)
{
    float4 d = texel_size.xyxy * float4(-1.0f, -1.0f, 1.0f, 1.0f);

    float4 s1 = source_texture.Sample(bilinear_sampler, uv + d.xy);
    float4 s2 = source_texture.Sample(bilinear_sampler, uv + d.zy);
    float4 s3 = source_texture.Sample(bilinear_sampler, uv + d.xw);
    float4 s4 = source_texture.Sample(bilinear_sampler, uv + d.zw);

    //Luma weight average
    float s1w = 1.0f / (Luminance(s1) + 1.0f);
    float s2w = 1.0f / (Luminance(s2) + 1.0f);
    float s3w = 1.0f / (Luminance(s3) + 1.0f);
    float s4w = 1.0f / (Luminance(s4) + 1.0f);
    float sum = 1.0f / (s1w + s2w + s3w + s4w);

    return (s1 * s1w + s2 * s2w + s3 * s3w + s4 * s4w) * sum;
}

//================================================================
//Upsample with a 4x4 box filter
//================================================================
float4 UpsampleBox
(
    float2 uv,
    float2 texel_size,
    Texture2D source_texture,
    SamplerState bilinear_sampler,
    float4 sample_scale
)
{
    float4 d = texel_size.xyxy * float4(-1.0f, -1.0f, 1.0f, 1.0f) * (sample_scale * 0.5f);

    float4 s;
    s  = source_texture.Sample(bilinear_sampler, uv + d.xy);
    s += source_texture.Sample(bilinear_sampler, uv + d.zy);
    s += source_texture.Sample(bilinear_sampler, uv + d.xw);
    s += source_texture.Sample(bilinear_sampler, uv + d.zw);

    return s * (1.0f / 4.0f);
}