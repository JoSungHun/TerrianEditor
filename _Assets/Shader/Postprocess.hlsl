#include "Common.hlsl"
#include "Blur.hlsl"
#include "Scaling.hlsl"

PixelTexture VS(VertexTexture input)
{
    PixelTexture output;

    output.position = mul(input.position, global_vp_ortho);
    output.uv       = input.uv;

    return output;
}

cbuffer BlurBuffer : register(b1)
{
    float2 blur_direction;
    float blur_sigma;
};

Texture2D source_texture1 : register(t0);
Texture2D source_texture2 : register(t1);
Texture2D source_texture3 : register(t2);
SamplerState samp : register(s0);

float4 PS(PixelTexture input) : SV_Target
{
    float4 color = float4(1, 0, 0, 1);

#if PASS_GAMMA_CORRECTION
    color = source_texture1.Sample(samp, input.uv);
    color = Gamma(color);
#endif

#if PASS_TEXTURE
    color = source_texture1.Sample(samp, input.uv);
#endif

#if PASS_DOWNSAMPLE_BOX
    color = DownsampleBox_AntiFlicker
    (
        input.uv, 
        global_texel_size, 
        source_texture1, 
        samp
    );
#endif

#if PASS_UPSAMPLE_BOX
    color = UpsampleBox
    (
        input.uv,
        global_texel_size,
        source_texture1,
        samp,
        4.0f
    );
#endif

#if PASS_BOX_BLUR
    color = BoxBlur
    (
        input.uv,
        global_texel_size,
        blur_sigma,
        source_texture1, 
        samp
    );
#endif

#if PASS_GAUSSIAN_BLUR
    color = GaussianBlur
    (
        input.uv,
        source_texture1,
        samp,
        global_texel_size,
        blur_direction,
        blur_sigma
    );
#endif

#if PASS_GAUSSIAN_BILATERAL_BLUR
    color = GaussianBilateralBlur
    (
        input.uv,
        source_texture1,
        source_texture2,
        source_texture3,
        samp,
        global_texel_size,
        blur_direction,
        blur_sigma
    );
#endif

#if PASS_BRIGHT
    color = source_texture1.Sample(samp, input.uv);
    color = Luminance(color) * color;
#endif

#if PASS_BLEND
    float4 source_color1 = source_texture1.Sample(samp, input.uv);
    float4 source_color2 = source_texture2.Sample(samp, input.uv);
    color = source_color1 + (source_color2 * global_bloom_intensity);
#endif

#if PASS_LUMA
    color   = source_texture1.Sample(samp, input.uv);
    color.a = Luminance(color);
#endif

#if PASS_MOTION_BLUR

#endif

    return color;
}