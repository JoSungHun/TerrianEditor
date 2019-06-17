cbuffer CameraBuffer : register(b0)
{
	matrix View;
	matrix Proj;
};

cbuffer LightBuffer : register(b1)
{
    matrix lightView;
    matrix lightProj;
    float3 lightPos;
};

struct VertexInput
{
	float4 Position : POSITION0;
    float2 Uv : TEXCOORD0;
    float4x4 worldTransform : WORLD;
	float4 Color : COLOR0;
};

struct PixelInput
{
	float4 Position : SV_POSITION0;
    float2 Uv : TEXCOORD0;
	float4 Color : COLOR0;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

PixelInput VS(VertexInput input)
{
	PixelInput output;
    input.Position.w = 1.0f;
	output.Position = mul(input.Position, input.worldTransform);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Proj);

    output.Uv = input.Uv;
	output.Color = input.Color;

    //광원기준 VERTEX 위치 계산
    output.lightViewPosition = mul(input.Position, input.worldTransform);
    output.lightViewPosition = mul(output.lightViewPosition, lightView);
    output.lightViewPosition = mul(output.lightViewPosition, lightProj);

    float4 worldPosition = mul(input.Position, input.worldTransform);

    output.lightPos = lightPos.xyz - worldPosition.xyz;

    output.lightPos = normalize(output.lightPos);

	return output;
}
