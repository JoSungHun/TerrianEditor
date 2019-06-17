
cbuffer CameraBuffer : register(b0)
{
	matrix View;
	matrix Proj;
};

cbuffer TransformBuffer : register(b1)
{
    matrix World;
};

cbuffer ColorBuffer : register(b2)
{
	float4 Color;
};

struct VertexInput
{
    float4 Position : POSITION0;
	float2 Uv : UV0;
	float4 Color : COLOR0;
};

struct PixelInput
{
    float4 Position : SV_POSITION0;
	float2 Uv : UV0;
	float4 Color :COLOR0;
};

PixelInput VS(VertexInput input)
{
	PixelInput output;
	output.Position = mul(input.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Proj);
	output.Uv = input.Uv;
	output.Color = input.Color;

	return output;
}

Texture2D SourceTexture : register(t0);
SamplerState Samp : register(s0);



float4 PS(PixelInput input) : SV_Target
{
	return input.Color;
}

