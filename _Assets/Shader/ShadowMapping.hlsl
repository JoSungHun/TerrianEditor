#include "Common.hlsl"

#define CASCADES 3
#define PCF_SAMPLES 2
#define PCF_DIM float(PCF_SAMPLES) / 2.0f

PixelTexture VS(VertexTexture input)
{
    PixelTexture output;

    output.position = mul(input.position, global_wvp);
    output.uv       = input.uv;

    return output;
}

Texture2D normal_texture                    : register(t0);
Texture2D depth_texture                     : register(t1);
Texture2DArray directional_depth_texture    : register(t2);
TextureCube point_depth_texture             : register(t3);
Texture2D spot_depth_texture                : register(t4);

SamplerComparisonState compare_depth        : register(s0);
SamplerState linear_clamp                   : register(s1);

cbuffer ShadowMapBuffer : register(b1)
{
    matrix view;
    matrix view_projection_inv;
    matrix view_projection[CASCADES];
    float3 positioin;
    float resolution;
    float3 direction;
    float range;
    float2 biases;
};

float Directional_DepthTest(float slice, float2 texcoord, float compare)
{
    return directional_depth_texture.SampleCmpLevelZero(compare_depth, float3(texcoord, slice), compare);
}

float Point_DepthTest(float3 direction, float compare)
{
    return point_depth_texture.SampleCmp(compare_depth, direction, compare);
}

float Spot_DepthTest(float2 texcoord, float compare)
{
    return spot_depth_texture.SampleCmp(compare_depth, texcoord, compare);
}

//Percentage Closer Filtering
float PCF_2D(int cascade, float texel, float2 texcoord, float compare)
{
    float amount_limit  = 0.0f;
    float count         = 0.0f;

    [unroll]
    for (float y = -PCF_DIM; y <= PCF_DIM; y++)
    {
        [unroll]
        for (float x = -PCF_DIM; x <= PCF_DIM; x++)
        {
            float2 offset = float2(x, y) * texel;

#if DIRECTIONAL
            amount_limit += Directional_DepthTest(cascade, texcoord + offset, compare);
#elif SPOT
            amount_limit += Spot_DepthTest(texcoord + offset, compare);
#endif
            count++;
        }
    }
    return amount_limit /= count;
}

float PCF_3D(float texel, float3 sample_direction, float compare)
{
    float amount_limit  = 0.0f;
    float count         = 0.0f;

    [unroll]
    for (float x = -PCF_DIM; x <= PCF_DIM; x++)
    {
        [unroll]
        for (float y = -PCF_DIM; y <= PCF_DIM; y++)
        {
            [unroll]
            for (float z = -PCF_DIM; z <= PCF_DIM; z++)
            {
                float3 offset = float3(x, y, z) * texel;
                amount_limit += Point_DepthTest(sample_direction + offset, compare);
                count++;
            }
        }
    }
    return amount_limit /= count;
}

float Directional_ShadowMapping(int cascade, float4 positionCS, float texel, float bias)
{
    if (positionCS.x < -1.0f || positionCS.x > 1.0f ||
        positionCS.y < -1.0f || positionCS.y > 1.0f ||
        positionCS.z < 0.0f || positionCS.z > 1.0f)
        return 1.0f;

    float2 texcoord = Project(positionCS);
    float cmp_depth = positionCS.z + bias;

    return PCF_2D(cascade, texel, texcoord, cmp_depth);
}

float PS(PixelTexture input) : SV_Target
{
    float2 texcoord             = input.uv;
    float3 normal               = normal_texture.Sample(linear_clamp, texcoord).rgb;
    float depth                 = depth_texture.Sample(linear_clamp, texcoord).r;
    float bias                  = biases.x;
    float normal_bias           = biases.y;
    float texel                 = 1.0f / resolution;
    float NdotL                 = dot(normal, direction);
    float cos_angle             = saturate(1.0f - NdotL);
    float3 scaled_normal_offset = normal * normal_bias * cos_angle * texel;
    float4 position_world       = float4(GetWorldPositionFromDepth(depth, view_projection_inv, texcoord) + scaled_normal_offset, 1.0f);

    //float4 positionCS[CASCADES];
    //positionCS[0] = mul(posi)

#if DIRECTIONAL
 
#elif POINT

#elif SPOT

#endif

    return 1.0f;
}