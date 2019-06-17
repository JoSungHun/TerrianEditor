cbuffer CameraBuffer : register(b0)
{
    matrix View;
    matrix Proj;
};

cbuffer TransformBuffer : register(b1)
{
    matrix World;
};

cbuffer LightBuffer : register(b2)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
}

struct VertexInput
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
};

struct PixelInput
{
    float4 Position : SV_POSITION0;
    float4 Color : COLOR0;
};

PixelInput VS(VertexInput input)
{
    PixelInput output;
    output.Position = mul(input.Position, World);
    output.Position = mul(output.Position, View);
    output.Position = mul(output.Position, Proj);

    output.Color = input.Color;

    return output;
}

float4 PS(PixelInput input) : SV_Target
{
    return input.Color;
}