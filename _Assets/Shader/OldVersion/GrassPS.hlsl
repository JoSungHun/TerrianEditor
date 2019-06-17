cbuffer LightData : register(b0)
{
    float4 ambient;
    float4 diffuse;
    float3 lightDirection;
};

struct PixelInput
{
	float4 Position : SV_POSITION0;
    float2 Uv : TEXCOORD0;
    float4 Color : COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

Texture2D SampTexture : register(t0);
Texture2D DepthMap : register(t1);
SamplerState samp : register(s0);

float4 PS(PixelInput input) : SV_Target
{
    float bias;
    float4 color;
    float2 projUv;
    float depthValue;
    float lightDepthValue;
    float lightIntensity;
    float4 texColor;
    float3 lightDir;

    lightDir = -lightDirection;

    bias = 0.00001f;

    color = ambient;

    projUv.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
    projUv.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;

    if ((saturate(projUv.x) == projUv.x) && (saturate(projUv.y) == projUv.y))
    {
        depthValue = DepthMap.Sample(samp, projUv).r;

        lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

        lightDepthValue = lightDepthValue - bias;

        if (lightDepthValue < depthValue)
        {
            color += diffuse;

            color = saturate(color);
        }
    }
    else
    {
        color += diffuse;
        color = saturate(color);
    }

    texColor = SampTexture.Sample(samp, input.Uv);

    color = color * texColor;

    return color;

}